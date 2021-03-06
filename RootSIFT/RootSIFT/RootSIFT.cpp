// RootSIFT.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "alg.h"
#include "iofiles.h"
#include "cvutils.h"
#include "utils.h"

#include <iostream>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <sstream>

#include <opencv2/core.hpp>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;



void outputprocessing(const wchar_t *filename);
//automatically add '\n'
void outputmsg(const std::wstring &str);

static void workthread() noexcept;
static bool getnextfile(
	fs::path &ifilepath,
	fs::path &siftpath,
	fs::path &kppath);


const wchar_t *helptext =
LR"__(usage: 
    RootSIFT [options] -i input1 [-fi filter1] [-o output1] -i input2 [-fi filter2] [-o output2] ...


options:
    -j n      the number of threads to use, n is an integer.
              (default: 1)
    -f        overwrite existing files.
              zero-length files will be treated as non-existing
              files.
    --od      orientation-dependent mode. the output SIFT descriptors
              will be sensitive to the orientation of the original
              image.
    --dr      draw keypoints of the original image. 
    --so      SIFT only, without hellinger normalization.
   
input:	      input image file name or directory name.
   
filter:       a regular expression (ECMAScript syntax) for files in
              "input". for example, to process ".jpg" files only,
              use ".+\.jpg".
              (default: process all the subfiles)
   
output:       output must be a directory name. (format: csv, where
              features are stored as matrix ROWS)
              if the output directory doesn't exist, please use a 
              trailing "\", for example, ".\results\", otherwise it
              will be treated as a file.
              (default: current directory)

)__";



static iofiles files;
static std::mutex mutex_getnextfile;

static int _j = 1;
static bool _od = false;
static bool _f = false;
static bool _dr = false;
static bool _so = false;

int _tmain(int argc, _TCHAR* argv[])
{
	int state = 0;
	//0: judge -j --od --dr -f -i
	//1: -j
	//2: -i

	//5: judge -o -fi -i (end state can be 5)
	//6: -fi
	//7: -o
	//8: -i

	const wchar_t *input = nullptr;
	const wchar_t *filter = nullptr;
	const wchar_t *output = nullptr;

	for (int i = 1; i < argc; i++) {
		wchar_t *arg = argv[i];
		wchar_t *stop;
		long tmp;
		float tmp2;
		bool ret;
		std::wstring errdescr;

		switch (state)
		{
		case 0:
			if (wcscmp(arg, L"-j") == 0) {
				state = 1;
				continue;
			}
			if (wcscmp(arg, L"-f") == 0) {
				state = 0;
				_f = true;
				continue;
			}
			if (wcscmp(arg, L"--od") == 0) {
				state = 0;
				_od = true;
				continue;
			}
			if (wcscmp(arg, L"--dr") == 0) {
				state = 0;
				_dr = true;
				continue;
			}
			if (wcscmp(arg, L"--so") == 0) {
				state = 0;
				_so = true;
				continue;
			}
			if (wcscmp(arg, L"-i") == 0) {
				state = 2;
				continue;
			}
			std::wcout << helptext;
			return 1;

		case 1:
			tmp = (int)wcstol(arg, &stop, 10);
			if (*stop == '\0' && tmp > 0) {
				state = 0;
				_j = tmp;
				continue;
			}
			std::wcout << helptext;
			return 1;

		case 2:
			state = 5;
			input = arg;
			continue;

		case 5:
			if (wcscmp(arg, L"-fi") == 0) {
				state = 6;
				continue;
			}
			if (wcscmp(arg, L"-o") == 0) {
				state = 7;
				continue;
			}
			if (wcscmp(arg, L"-i") == 0) {
				state = 8;
				continue;
			}
			std::wcout << helptext;
			return 1;

		case 6:
			state = 5;
			filter = arg;
			if (checkregex(filter, errdescr)) {
				continue;
			}
			std::wcout << L"Error in expression \"" << filter << "\": " << errdescr << '\n';
			return 1;

		case 7:
			state = 5;
			output = arg;
			continue;

		case 8:
			state = 5;
			if (filter == nullptr) {
				filter = L"";
			}
			if (output == nullptr) {
				output = L".";
			}

			if (!files.addpath(input, output, filter, true)) {
				std::wcout << helptext;
				return 1;
			}

			input = arg;
			filter = nullptr;
			output = nullptr;
			continue;

		default:
			std::wcout << helptext;
			return 1;
			break;
		}
	} //for

	if (state != 5) {
		std::wcout << helptext;
		return 1;
	}

	// state==5, no more -i found
	if (filter == nullptr) {
		filter = L"";
	}
	if (output == nullptr) {
		output = L".";
	}
	if (!files.addpath(input, output, filter, true)) {
		std::wcout << helptext;
		return 1;
	}

	std::vector<std::thread> threads;
	for (int i = 0; i < _j; i++) {
		threads.push_back(std::thread(workthread));
	}

	for (int i = 0; i < _j; i++) {
		threads[i].join();
	}

	//workthread();

	std::wcout << L"\nFinished!\n";

	return 0;
}


static void workthread() noexcept
{
	fs::path ifilepath, siftpath, kppath;

	std::wstring errmsg;

	while (getnextfile(ifilepath, siftpath, kppath)) {

		cv::Mat img, kpimg;
		std::vector<cv::KeyPoint> keypoints;
		cv::Mat descriptors;

		if (!_f) {        //don't overwrite existing files
			if (
				fileexists(siftpath) &&
				(fileexists(kppath)||!_dr)
				) {
				outputmsg(std::wstring() + L"Skipped: " + ifilepath.filename().c_str());
				continue;
			}
		}

		int phase = 0;

		try {
			outputprocessing(ifilepath.filename().c_str());

			phase = 1; errmsg = std::wstring() + L"Error reading " + ifilepath.native();
			if (!readimgfile(ifilepath.wstring().c_str(), img)) {
				goto errproc;
			}

			phase = 2; errmsg = std::wstring() + L"Error: computesift";
			std::tie(keypoints, descriptors) = computesift(img, _od);

			if (_dr) {
				phase = 3; errmsg = std::wstring() + L"Error: drawkeypoints";
				kpimg = drawkeypoints(img, keypoints);

				phase = 4; errmsg = std::wstring() + L"Error writing " + kppath.native();
				if (!writeimgfile(kppath.wstring().c_str(), kpimg)) {
					goto errproc;
				}
			}

			if (!_so) {
				phase = 5; errmsg = std::wstring() + L"Error: hellinger_norm";
				hellinger_norm(descriptors);
			}

			phase = 6; errmsg = std::wstring() + L"Error writing " + siftpath.native();
			if (!mat2csv(siftpath.wstring().c_str(), descriptors)) {
				goto errproc;
			}

		}
		catch (...) {
			goto errproc;
		}

		continue;

	errproc:
		outputmsg(errmsg);
		continue;
	}

}


static bool getnextfile(
	fs::path &ifilepath,
	fs::path &siftpath,
	fs::path &kppath)
{
	std::unique_lock<std::mutex> lck(mutex_getnextfile);

	fs::path opath;
	if (!files.getnextfile(ifilepath, opath)) {
		return false;
	}

	std::wstring origfilename = ifilepath.filename().wstring();

	siftpath = opath;
	siftpath.append(origfilename + L".csv");

	if (_dr) {
		kppath = opath;
		kppath.append(origfilename + L".jpg");
	}

	return true;

}













static std::mutex outputmutex;
static int lastlength = 0;
static std::wstring laststr;
void outputprocessing(const wchar_t *filename)
{
	static int n = 0;
	std::unique_lock<std::mutex> lck(outputmutex);
	n++;

	std::wcout << backspace(lastlength);

	std::wostringstream str;
	str << L"Processing: " << n << L"   " << filename;
	std::wcout << str.str();

	lastlength = str.str().length();
	laststr = str.str();

}

//automatically add '\n'
void outputmsg(const std::wstring &str)
{
	std::unique_lock<std::mutex> lck(outputmutex);

	std::wcout << backspace(lastlength);

	std::wcout << L"    " << str << '\n';

	std::wcout << laststr;
}