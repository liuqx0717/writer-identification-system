// PCA-Wh.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "utils.h"
#include "cvutils.h"
#include "alg.h"
#include "iofiles.h"

#include <mutex>
#include <memory>
#include <thread>
#include <vector>
#include <iostream>
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
	fs::path &ofilepath);




const wchar_t *helptext =
LR"__(usage: 
    PCA-Wh [options] -i input1 [-fi filter1] [-o output1] -i input2 [-fi filter2] [-o output2] ...

options:
    -j n      the number of threads to use, n is an integer.
              (default: 1)
    -k n      the number of eigenvectors to keep,  n is an integer.
              set n to 0 if you want to keep all the eigenvectors.
              (default: 0)
    -f        overwrite existing files.
              zero-length files will be treated as non-existing
              files.
    --po      PCA only, without whitening.
    --nozm    no zero-mean mode, process the data directly without
              ensuring that the data has zero-mean.
   
input:	      input file name or directory name. (format: csv, where
              features are stored as matrix ROWS)
   
filter:       a regular expression (ECMAScript syntax) for files in
              "input". for example, to process ".csv" files only,
              use ".+\.csv".
              (default: process all the subfiles)
   
output:       output must be a directory name.
              if the output directory doesn't exist, please use a 
              trailing "\", for example, ".\results\", otherwise it
              will be treated as a file.
              (default: current directory)

)__";




static iofiles files;
static std::mutex mutex_getnextfile;

static int _j = 1;
static int _k = 0;
static bool _f = false;
static bool _po = false;
static bool _nozm = false;

int _tmain(int argc, _TCHAR* argv[])
{
	int state = 0;
	//0: judge -j -k -f --po --nozm -i
	//1: -j
	//2: -i
	//3: -k

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
		std::wstring errdescr;

		switch (state)
		{
		case 0:
			if (wcscmp(arg, L"-j") == 0) {
				state = 1;
				continue;
			}
			if (wcscmp(arg, L"-k") == 0) {
				state = 3;
				continue;
			}
			if (wcscmp(arg, L"-f") == 0) {
				state = 0;
				_f = true;
				continue;
			}
			if (wcscmp(arg, L"--po") == 0) {
				state = 0;
				_po = true;
				continue;
			}
			if (wcscmp(arg, L"--nozm") == 0) {
				state = 0;
				_nozm = true;
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

		case 3:
			tmp = (int)wcstol(arg, &stop, 10);
			if (*stop == '\0' && tmp >= 0) {
				state = 0;
				_k = tmp;
				continue;
			}
			std::wcout << helptext;
			return 1;

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

#ifndef _DEBUG
	std::vector<std::thread> threads;
	for (int i = 0; i < _j; i++) {
		threads.push_back(std::thread(workthread));
	}

	for (int i = 0; i < _j; i++) {
		threads[i].join();
	}
#endif

#ifdef _DEBUG
	workthread();
#endif

	std::wcout << L"\nFinished!\n";

	return 0;
}


static void workthread() noexcept
{
	fs::path ifilepath, ofilepath;

	std::wstring errmsg;

	while (getnextfile(ifilepath, ofilepath)) {

		cv::Mat data;
		cv::Mat data_rot;
		cv::Mat data_out;
		cv::Mat S;

		if (!_f) {        //don't overwrite existing files
			if (
				fileexists(ofilepath)
				) {
				outputmsg(std::wstring() + L"Skipped: " + ifilepath.filename().c_str());
				continue;
			}
		}

		int phase = 0;

#ifndef _DEBUG
		try {
#endif
			outputprocessing(ifilepath.filename().c_str());

			phase = 1; errmsg = std::wstring() + L"Error reading " + ifilepath.native();
			if (!csv2mat32f(ifilepath.wstring().c_str(), data)) {
				goto errproc;
			}
			cv::transpose(data, data);

			if (!_nozm) {
				phase = 2; errmsg = std::wstring() + L"Error: zero_mean";
				zero_mean(data);

			}

			phase = 3; errmsg = std::wstring() + L"Error: pca";
			data_rot = pca(data, _k, S);

			if (!_po) {
				phase = 4; errmsg = std::wstring() + L"Error: pca_wh";
				pca_wh(data_rot, S);
			}

			phase = 5; errmsg = std::wstring() + L"Error writing " + ofilepath.native();
			cv::transpose(data_rot, data_out);
			if (!mat2csv(ofilepath.wstring().c_str(), data_out)) {
				goto errproc;
			}
			
#ifndef _DEBUG
		}
		catch (...) {
			goto errproc;
		}
#endif

		continue;

	errproc:
		outputmsg(errmsg);
		continue;
	}

}


static bool getnextfile(
	fs::path &ifilepath,
	fs::path &ofilepath)
{
	std::unique_lock<std::mutex> lck(mutex_getnextfile);

	fs::path opath;
	if (!files.getnextfile(ifilepath, opath)) {
		return false;
	}

	std::wstring origfilename = ifilepath.filename().wstring();

	ofilepath = opath;
	ofilepath.append(origfilename + L".csv");

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