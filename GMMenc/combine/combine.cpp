// combine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "iofiles.h"

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <memory>

#include <boost/filesystem.hpp>





const wchar_t *backspace(int n);
void outputmsg(const std::wstring &str);
void outputprocessing(const wchar_t *filename);
bool getlabel(const std::wstring &filename, char *out);


const wchar_t *helptext =
LR"__(usage: 
    combine input [output]

input:    input directory.

output:   output directory.
          (default: current directory)

)__";


static iofiles files;

int _tmain(int argc, _TCHAR* argv[])
{
	namespace fs = boost::filesystem;

	const wchar_t *arg1 = L".", *arg2 = L".";

	if (argc == 2) {
		arg1 = argv[1];
	}
	else if (argc == 3) {
		arg1 = argv[1];
		arg2 = argv[2];
	}
	else {
		std::wcout << helptext;
		return 1;
	}

	fs::path inputpath(arg1), outputpath(arg2);
	fs::path odatafilepath(outputpath); odatafilepath.append(L"data.csv");
	fs::path olablesfilepath(outputpath); olablesfilepath.append(L"labels.csv");

	if (!fs::is_directory(inputpath)) {
		std::wcout << helptext;
		return 1;
	}

	if (!files.addpath(inputpath, outputpath, LR"__(.+\.csv)__", true)) {
		std::wcout << helptext;
		return 1;
	}

	std::fstream odatafile(odatafilepath.wstring().c_str(), std::fstream::out | std::fstream::trunc);
	if (!odatafile) {
		outputmsg(L"Error writing data.csv");
	}

	std::fstream olablesfile(olablesfilepath.wstring().c_str(), std::fstream::out | std::fstream::trunc);
	if (!odatafile) {
		outputmsg(L"Error writing labels.csv");
	}


	fs::path ipath, opath;
	char idstr[32];
	const size_t linebuffsize = 4 * 1024 * 1024;
	std::unique_ptr<char[]> linebuff(new char[linebuffsize]);
	char *line = linebuff.get();
	size_t linelength = 0;

	while (files.getnextfile(ipath, opath)) {
		outputprocessing(ipath.filename().c_str());

		std::wstring filename = ipath.filename().wstring();
		if (!getlabel(filename, idstr)) {
			outputmsg(std::wstring() + L"Skipped: " + filename);
			continue;
		}

		std::fstream ifile(ipath.wstring(), std::fstream::in);
		if (!ifile) {
			outputmsg(std::wstring() + L"Error reading " + ipath.native().c_str());
		}
		ifile.getline(line, linebuffsize);
		linelength = ifile.gcount();

		odatafile.write(line, linelength);
		odatafile.put('\n');
		olablesfile.write(idstr, strlen(idstr));
		olablesfile.put('\n');

	}

	std::wcout << L"\nFinished!\n";

    return 0;
}


bool getlabel(const std::wstring &filename, char *out)
{
	int id;

	size_t pos = filename.find_first_of('-');
	if (pos == std::wstring::npos) {
		return false;
	}

	std::wstring tmp = filename.substr(0, pos);
	wchar_t *stop;
	id = (int)wcstol(tmp.c_str(), &stop, 10);
	if (*stop != '\0') {
		return false;
	}

	_itoa(id, out, 10);

	return true;
}









static int lastlength = 0;
static std::wstring laststr;
void outputprocessing(const wchar_t *filename)
{
	static int n = 0;
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

	std::wcout << backspace(lastlength);

	std::wcout << L"    " << str << '\n';

	std::wcout << laststr;
}


//output backspace
const wchar_t *backspace(int n)
{
	const int buffersize = 128;
	static wchar_t str[buffersize];
	static int lastn = 0;
	static bool init = false;

	if (!init) {
		init = true;

		for (int i = 0; i < buffersize; i++) {
			str[i] = '\b';
		}
	}

	if (n < buffersize) {
		str[lastn] = '\b';
		str[n] = '\0';
		lastn = n;
	}

	return str;
}