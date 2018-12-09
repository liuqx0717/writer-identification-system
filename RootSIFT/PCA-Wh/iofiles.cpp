#include "stdafx.h"

#include "iofiles.h"

#include <list>
#include <regex>

#include <boost/filesystem.hpp>


// ipath must exist, otherwise this function will return false.
// when ipath is a directory, you can use regex as a filter of the subfiles
// if ipath is a file, opath can be one of the following:
//    an existing file name
//    an existing directory name
//    a non-existing file name
//    a non-existing directory name with a trailing "/" or "\"
// if ipath is a directory, opath can be either of the following:
//    an existing directory name
//    a non-existing directory name
// this function will create the non-existing directory, if that fails, this function will return false.
bool iofiles::addpath(
	const boost::filesystem::path &ipath,
	const boost::filesystem::path &opath,
	const std::wstring regex,
	bool output_must_be_directory)
{
	using namespace boost::filesystem;


	//symlink_status: follow symbol links
	//status: just return the type of the file, won't follow symbol links.
	file_type itype = symlink_status(ipath).type();
	file_type otype = symlink_status(opath).type();

	if (itype == file_not_found) {
		return false;
	}
	else if (itype == directory_file) {
		_iofolder iofolder;
		iofolder.ifolder = directory_iterator(ipath);
		if (regex.length() != 0) {
			iofolder.has_regex = true;
			iofolder.regex.assign(regex);
		}
		else {
			iofolder.has_regex = false;
		}

		if (otype == file_not_found) {
			bool ret = create_directory(opath);
			if (!ret) return false;

			iofolder.ofolder = opath;
			folders.push_back(std::move(iofolder));
			return true;
		}
		else if (otype == directory_file) {
			iofolder.ofolder = opath;
			folders.push_back(std::move(iofolder));
			return true;
		}
		else {
			return false;
		}

	}
	else if (itype != status_error) {
		_iofile iofile;
		iofile.ifile = ipath;

		if (otype == file_not_found) {
			wchar_t trailing_char = opath.native().back();
			if (trailing_char == '\\' || trailing_char == '/') {    //opath is a non-existing directory
				bool ret = create_directory(opath);
				if (!ret) return false;
			}
			else {                                                  //opath is a non-existing file
				if (output_must_be_directory) {
					return false;
				}
			}


			iofile.ofile = opath;
			files.push_back(std::move(iofile));
			return true;
		}
		else if (otype != status_error) {
			iofile.ofile = opath;
			files.push_back(std::move(iofile));
			return true;
		}
		else {
			return false;
		}

	}
	else {
		return false;
	}


}

// output:
//    ifilepath: input file path, which will always be a file.
//    opath: output path, which will be either a file or a directory.
// return value:
//    return false if there is no more file.
bool iofiles::getnextfile(boost::filesystem::path &ifilepath, boost::filesystem::path &opath)
{
	using namespace boost::filesystem;

	if (!files.empty()) {      // if "files" is not empty
		auto i = files.begin();
		ifilepath = std::move((*i).ifile);
		opath = std::move((*i).ofile);
		files.erase(i);
		return true;
	}

	while (!folders.empty()) {   // if "folders" is not empty
		auto i = folders.begin();
		std::wregex &regex = (*i).regex;
		bool has_regex = (*i).has_regex;

		while (1) {
			directory_iterator &subfile = (*i).ifolder;


			if (subfile == directory_iterator()/*end iterator*/) {
				break;
			}

			path subfilepath = (*subfile).path();
			//symlink_status: follow symbol links
			//status: just return the type of the file, won't follow symbol links.
			file_type subfiletype = symlink_status(subfilepath).type();

			if (subfiletype == directory_file || subfiletype == status_error) {
				subfile++;
				continue;
			}

			if (has_regex) {
				if (!std::regex_match(subfilepath.filename().c_str(), regex)) {
					subfile++;
					continue;
				}
			}
			

			ifilepath = subfilepath;
			opath = (*i).ofolder;
			subfile++;
			return true;
		}

		folders.erase(i);
		continue;

	}


	return false;

}



#ifdef _DEBUG

#include <iostream>

void test_iofiles()
{
	namespace fs = boost::filesystem;

	iofiles f;
	f.addpath(L"C:\\FTP_Shared\\tmp\\", L"C:\\FTP_Shared\\tmp", LR"__(.*\.bat)__",false);
	f.addpath(L"E:", L"E:\\test\\",false);
	f.addpath(L"D:\\cvl-database-cropped-1-1\\0001-1-cropped.tif", L"D:\\test\\",false);

	fs::path ip, op;
	while (f.getnextfile(ip, op)) {
		std::wcout << ip.native() << '\n' << op.native() << '\n';
	}
}

#endif