#include "stdafx.h"

#include "cvutils.h"
#include "utils.h"
#include "enc.h"
#include "iofiles.h"

#include <vector>

#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>

#include <opencv2/core.hpp>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;


static bool getnextfile(
	fs::path &ifilepath,
	fs::path &miupath,
	fs::path &pipath,
	fs::path &sigmapath,
	fs::path &gammapath);

static void workthread() noexcept;
void outputprocessing(const wchar_t *filename);
//automatically add '\n'
void outputmsg(const std::wstring &str);


extern const wchar_t *helptext;

static iofiles files;
static std::mutex mutex_getnextfile;

static int _j = 1;
static int _k = 50;
static bool _f = false;

int main_em(int argc, _TCHAR* argv[])
{
	int state = 0;
	//0: judge -k -j -i
	//1: -j
	//2: -k
	//4: -i

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
		bool ret;
		std::wstring errdescr;

		switch (state)
		{
		case 0:
			if (wcscmp(arg, L"-j") == 0) {
				state = 1;
				continue;
			}
			if (wcscmp(arg, L"-k") == 0) {
				state = 2;
				continue;
			}
			if (wcscmp(arg, L"-f") == 0) {
				state = 0;
				_f = true;
				continue;
			}
			if (wcscmp(arg, L"-i") == 0) {
				state = 4;
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
			tmp = (int)wcstol(arg, &stop, 10);
			if (*stop == '\0' && tmp > 0) {
				state = 0;
				_k = tmp;
				continue;
			}
			std::wcout << helptext;
			return 1;

		case 4:
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

	return 0;

}


static void workthread() noexcept
{
	fs::path ifilepath;
	fs::path miupath;
	fs::path pipath;
	fs::path sigmapath;
	fs::path gammapath;

	std::wstring errmsg;
	
	while (getnextfile(ifilepath, miupath, pipath, sigmapath, gammapath))
	{
		if (!_f) {        //don't overwrite existing files
			if (
				fileexists(miupath) &&
				fileexists(pipath) && 
				fileexists(sigmapath) &&
				fileexists(gammapath)
				) {
				outputmsg(std::wstring() + L"Skipped: " + ifilepath.filename().native());
				continue;
			}
		}



		cv::Mat samples;
		cv::Mat miu;
		cv::Mat pi;
		cv::Mat sigma;
		cv::Mat gamma;

		int phase = 0;

		try {
			outputprocessing(ifilepath.filename().c_str());

			phase = 1; errmsg = std::wstring() + L"Error reading " + ifilepath.native();
			if (!csv2mat32f(ifilepath.c_str(), samples)) {
				goto errproc;
			}

			phase = 2; errmsg = std::wstring() + L"Error: gmm_em";
			if (!gmm_em(samples, _k, miu, pi, sigma, gamma)) {
				goto errproc;
			}

			phase = 3; errmsg = std::wstring() + L"Error writing " + miupath.native();
			if (!mat2csv(miupath.c_str(), miu)) {
				goto errproc;
			}

			phase = 4; errmsg = std::wstring() + L"Error writing " + pipath.native();
			if (!mat2csv(pipath.c_str(), pi)) {
				goto errproc;
			}

			phase = 5; errmsg = std::wstring() + L"Error writing " + sigmapath.native();
			if (!mat2csv(sigmapath.c_str(), sigma)) {
				goto errproc;
			}

			phase = 6; errmsg = std::wstring() + L"Error writing " + gammapath.native();
			if (!mat2csv(gammapath.c_str(), gamma)) {
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
	fs::path &miupath,
	fs::path &pipath,
	fs::path &sigmapath,
	fs::path &gammapath) 
{
	std::unique_lock<std::mutex> lck(mutex_getnextfile);
	
	fs::path opath;

	if (!files.getnextfile(ifilepath, opath)) {
		return false;
	}

	// p.filename() is of type path, and will be surrounded by quotes when implicitly converted, 
	// so you will get: filename and extension: "file.ext" You may want p.filename().string() instead
	std::wstring origfilepath = ifilepath.filename().wstring();

	miupath = opath;
	miupath.append(origfilepath + L".miu");

	pipath = opath;
	pipath.append(origfilepath + L".pi");

	sigmapath = opath;
	sigmapath.append(origfilepath + L".sigma");

	gammapath = opath;
	gammapath.append(origfilepath + L".gamma");

	return true;
	
}