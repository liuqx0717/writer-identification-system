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
	fs::path &samplepath,
	fs::path &miupath,
	fs::path &pipath,
	fs::path &sigmapath,
	fs::path &gammapath,
	fs::path &ofilepath);

static void workthread() noexcept;
void outputprocessing(const wchar_t *filename);
//automatically add '\n'
void outputmsg(const std::wstring &str);
static bool addsearchdirectory(const fs::path &path);


extern const wchar_t *helptext;

static iofiles files;
static std::mutex mutex_getnextfile;

static int _j = 1;
static float _r = 30.0F;
static bool _f = false;
static bool _emonly = false;
static std::vector<fs::path> searchdirectories;

//#pragma message("main_em")
int main_sv(int argc, _TCHAR* argv[])
{
	int state = 0;
	//0: judge -r -j -i -s --emonly
	//1: -j
	//2: -r
	//3: -s
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
			if (wcscmp(arg, L"-r") == 0) {
				state = 2;
				continue;
			}
			if (wcscmp(arg, L"-s") == 0) {
				state = 3;
				continue;
			}
			if (wcscmp(arg, L"-f") == 0) {
				state = 0;
				_f = true;
				continue;
			}
			if (wcscmp(arg, L"--emonly") == 0) {
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
			tmp2 = wcstof(arg, &stop);
			if (*stop == '\0' && tmp2 > 0) {
				state = 0;
				_r = tmp2;
				continue;
			}
			std::wcout << helptext;
			return 1;

		case 3:
			if (addsearchdirectory(arg)) {
				state = 0;
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

			if (!fs::is_directory(input)) {
				std::wcout << helptext;
				return 1;
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
	if (!fs::is_directory(input)) {
		std::wcout << helptext;
		return 1;
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


//#pragma message("workthread")
static void workthread() noexcept
{
	fs::path samplepath;
	fs::path miupath;
	fs::path pipath;
	fs::path sigmapath;
	fs::path gammapath;
	fs::path ofilepath;

	std::wstring errmsg;

	while (getnextfile(samplepath, miupath, pipath, sigmapath, gammapath, ofilepath))
	{
		if (!_f) {        //don't overwrite existing files
			if (fileexists(ofilepath)) {
				outputmsg(std::wstring() + L"Skipped: " + miupath.stem().native());
				continue;
			}
		}



		cv::Mat samples;
		cv::Mat miu;
		cv::Mat pi;
		cv::Mat sigma;
		cv::Mat gamma;

		cv::Mat adapted_miu, adapted_pi, adapted_sigma;
		cv::Mat norm_miu, norm_pi, norm_sigma;

		int phase = 0;

		try {
			outputprocessing(ofilepath.filename().c_str());

			phase = 1; errmsg = std::wstring() + L"Error reading " + samplepath.native();
			if (!csv2mat32f(samplepath.c_str(), samples)) {
				goto errproc;
			}


			phase = 2; errmsg = std::wstring() + L"Error reading " + miupath.native();
			if (!csv2mat32f(miupath.c_str(), miu)) {
				goto errproc;
			}

			phase = 3; errmsg = std::wstring() + L"Error reading " + pipath.native();
			if (!csv2mat32f(pipath.c_str(), pi)) {
				goto errproc;
			}

			phase = 4; errmsg = std::wstring() + L"Error reading " + sigmapath.native();
			if (!csv2mat32f(sigmapath.c_str(), sigma)) {
				goto errproc;
			}

			phase = 5; errmsg = std::wstring() + L"Error reading " + gammapath.native();
			if (!csv2mat32f(gammapath.c_str(), gamma)) {
				goto errproc;
			}

			cv::Mat sv;
			if (!_emonly) {
				phase = 6; errmsg = std::wstring() + L"Error: gmm_adapt";
				std::tie(adapted_miu, adapted_pi, adapted_sigma) =
					gmm_adapt(samples, _r, miu, pi, sigma, gamma);

				phase = 7; errmsg = std::wstring() + L"Error: gmm_normalize";
				std::tie(norm_miu, norm_pi, norm_sigma) =
					gmm_normalize(miu, pi, sigma, adapted_miu, adapted_pi, adapted_sigma);

				phase = 8; errmsg = std::wstring() + L"Error: mat2line";
				sv = mat2line(2, &norm_miu, &norm_sigma);
			}
			else {
				phase = 8; errmsg = std::wstring() + L"Error: mat2line";
				sv = mat2line(2, &miu, &sigma);
			}

			phase = 9; errmsg = std::wstring() + L"Error writing " + ofilepath.native();
			if (!mat2csv(ofilepath.c_str(), sv)) {
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

//#pragma message("getnextfile")
static bool getnextfile(
	fs::path &samplepath,
	fs::path &miupath,
	fs::path &pipath,
	fs::path &sigmapath,
	fs::path &gammapath,
	fs::path &ofilepath)
{
	std::unique_lock<std::mutex> lck(mutex_getnextfile);

	fs::path ifilename, opath;

	std::wstring tmp_origfilepath, tmp_pipath, tmp_sigmapath, tmp_gammapath;

	while (files.getnextfile(ifilename, opath)) {
		if (ifilename.extension().wstring().compare(L".miu") != 0) {
			continue;
		}

		miupath = ifilename;

		fs::path orig = miupath.parent_path();
		orig.append(miupath.stem().wstring());

		// p.filename() is of type path, and will be surrounded by quotes when implicitly converted, 
		// so you will get: filename and extension: "file.ext" You may want p.filename().string() instead
		tmp_origfilepath = orig.wstring();

		tmp_pipath = tmp_origfilepath + L".pi";
		tmp_sigmapath = tmp_origfilepath + L".sigma";
		tmp_gammapath = tmp_origfilepath + L".gamma";

		pipath = fs::path(tmp_pipath);
		sigmapath = fs::path(tmp_sigmapath);
		gammapath = fs::path(tmp_gammapath);

		if (fileexists(pipath) && fileexists(sigmapath) && fileexists(gammapath)) {
			std::wstring filename;

			if (fileexists(orig)) goto success;

			filename = orig.filename().wstring();
			for (auto dir : searchdirectories) {
				orig = dir;
				orig.append(filename);
				if (fileexists(orig)) goto success;
			}

			outputmsg(std::wstring() + L"Couldn't find original data file: " + orig.filename().native());
			continue;

		success:
			samplepath = orig;
			ofilepath = opath;
			ofilepath.append(orig.filename().wstring());

			return true;
		}

		continue;

	}



	return false;

}


//#pragma message("addsearchdirectory")
static bool addsearchdirectory(const fs::path &path)
{

	if (!fs::is_directory(path)) {
		return false;
	}

	searchdirectories.push_back(path);

	return true;
}


