#include "stdafx.h"

#include "cvutils.h"

#include <memory>
#include <fstream>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include <boost/filesystem.hpp>




bool readimgfile(const wchar_t *path, cv::Mat &out)
{
	//namespace fs = boost::filesystem;

	//uintmax_t size = fs::file_size(path);
	//if (size == static_cast<uintmax_t>(-1)) {
	//	return false;
	//}

	//std::unique_ptr<char[]> filebuff(new char[size]);
	//std::fstream file(path, std::fstream::binary | std::fstream::in);
	//if (!file) {
	//	return false;
	//}

	//file.read(filebuff.get(), size);

	//out = cv::imdecode(cv::_InputArray(filebuff.get(), size), cv::IMREAD_GRAYSCALE);

	//return true;

	const size_t tmpsize = 1024;
	char tmp[tmpsize];
	wcstombs(tmp, path, tmpsize);

	out = cv::imread(tmp, cv::IMREAD_GRAYSCALE);

	return out.data != nullptr;
	
}


bool writeimgfile(const wchar_t *path, const cv::Mat &in)
{
	const size_t tmpsize = 1024;
	char tmp[tmpsize];
	wcstombs(tmp, path, tmpsize);

	bool ret = cv::imwrite(tmp, in);
	return ret;

}





bool mat2csv(const wchar_t *filename, cv::Mat &mat, int significant_digits)
{
	if ((mat.type() & CV_MAT_DEPTH_MASK) == CV_32F) {
		return __matf2csv<float>(filename, mat, significant_digits);
	}
	else if ((mat.type() & CV_MAT_DEPTH_MASK) == CV_64F) {
		return  __matf2csv<double>(filename, mat, significant_digits);
	}
	else {
		return false;
	}

	return true;

}