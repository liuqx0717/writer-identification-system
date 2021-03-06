// GMMenc.cpp : Defines the entry point for the console application.
//

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
#include <sstream>

#include <opencv2/core.hpp>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

//const wchar_t *samplesfilename = L"C:\\FTP_Shared\\tmp\\results\\0001-1-cropped.tif.txt";


int main_em(int argc, _TCHAR* argv[]);
int main_sv(int argc, _TCHAR* argv[]);

const wchar_t *helptext =
LR"__(usage: 
    GMMenc command [options] -i input1 [-fi filter1] [-o output1] -i input2 [-fi filter2] [-o output2] ...

command:
    em   get GMM parameters (means, covs, weights). 
         the results will be stored in ".miu" ".sigma" ".pi" and 
         ".gamma" files (format: csv).

         options:
             -j n      the number of threads to use, n is an integer.
                       (default: 1)
             -k n      the number of components of GMM, n is an integer.
                       (default: 50)
             -f        overwrite existing files.
                       zero-length files will be treated as non-existing
                       files.
				       
         input:	       input file name or directory name (format: csv).
				       
         filter:       a regular expression (ECMAScript syntax) for files in
                       "input". for example, to process ".csv" files only,
                       use ".+\.csv".
                       (default: process all the subfiles)
				       
         output:       output must be a directory name.
                       if the output directory doesn't exist, please use a 
                       trailing "\", for example, ".\results\", otherwise it
                       will be treated as a file.
                       (default: current directory)

    sv   transform GMM parameters to supervectors.
         it will automatically search for corresponding ".miu" ".sigma" ".pi"
         ".gamma" and original data files.

         options:
             -j n      the number of threads to use, n is an integer.
                       (default: 1)
             -r r      the relevance factor, r is a float.
                       (default: 30)
             -s	path   additional directory to search for the
                       original data files.
                       the original data files are deducted by the
                       input ".miu" ".sigma" ".pi" and ".gamma" files.
                       you can specify multiple -s options.
                       (default: the same directory specified by "-i")
             -f        overwrite existing files.
                       zero-length files will be treated as non-existing
                       files.
             --emonly  use the results of em only, without adaptation and 
                       normalization.
				       
         input:	       input must be a directory name. it will automatically 
                       search for corresponding ".miu" ".sigma" ".pi" and
                       ".gamma" files.
				       
         filter:       a regular expression (ECMAScript syntax) for files in 
                       "input". subfiles (in "input") that don't match the 
                       pattern will be filtered out.
                       (default: process all the subfiles)
                       
         output:       output must be a directory name.
                       if the output directory doesn't exist, please use a 
                       trailing "\", for example, ".\results\", otherwise it 
                       will be treated as a file.
                       (default: current directory)

)__";







int _tmain(int argc, _TCHAR* argv[])
{
	int ret = 0;

	if (argc >= 4) {
		if (wcscmp(argv[1], L"em") == 0) {
			ret = main_em(argc - 1, argv + 1);
			goto finish;
		}
		if (wcscmp(argv[1], L"sv") == 0) {
			ret = main_sv(argc - 1, argv + 1);
			goto finish;
		}

		std::wcout << helptext;
		return 1;
	}
	else {
		std::wcout << helptext;
		return 1;
	}

finish:
	if (ret == 0) {
		std::wcout << L"\nFinished!\n";
#ifdef _DEBUG
		std::wcout << L"Press Enter to continue...";
		getchar();
#endif
	}

	return ret;

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

