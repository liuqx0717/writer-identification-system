#include "stdafx.h"

#include "cvutils.h"

#include <fstream>
#include <cstdarg>
#include <memory>

#include <opencv2/core.hpp>

//#include <iostream>


bool csv2mat32f(const wchar_t *filename, cv::Mat &result, int channel)
{
	std::fstream file(filename, std::fstream::in | std::fstream::_Nocreate);
	if (!file) {
		return false;
	}

	int rows = 0;
	int cols = 0;

	//int currentr = 0;
	int currentc = 0;

	const size_t linebuffersize = 200 * 1024;
	std::unique_ptr<char[]> linebuff(new char[linebuffersize]);
	char *line = linebuff.get();

	std::vector<float> data;

	while (!file.eof()) {
		//currentr++;
		currentc = 0;
		//std::cout << rows << '\n';
		file.getline(line, linebuffersize);

		for (size_t i = 0, j = 0; i < linebuffersize; i++) {
			char numstr[64];
			char *stopstr;
			char c = line[i];
			if (c == ' ' || c == '\t') {
				continue;
			}
			else if (c == ',' || c == '\n' || c == '\0') {
				if (j == 0) break;    //this is an empty line (not so strictly) 

				currentc++;

				numstr[j] = '\0';
				float num = strtod(numstr, &stopstr);
				if (*stopstr != '\0') return false;    //if there is an unrecognized char
				j = 0;

				data.push_back(num);
				if (c == '\0' || c == '\n') {
					break;
				}

				continue;
			}
			else {
				numstr[j] = c;
				j++;
				continue;
			}
		}  //for

		//currentc: the number of columns of current line
		if (currentc == 0) {  //empty line (not so strictly)
			continue;
		}
		else {
			if (cols == 0) {  //set cols of Mat
				cols = currentc;
				rows++;
				continue;
			}
			else if (cols == currentc) {  //the data of current line is valid
				rows++;
				continue;
			}
			else {                        //the data of current line is invalid
				return false;
			}
		}

	} //while

	cv::Mat(rows, cols / channel, CV_MAKETYPE(CV_32F, channel), data.data()).copyTo(result);   //step: Number of bytes each matrix row occupies


	return true;
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

// if copy is false, this function won't necessarily make a copy of input,
// which means that modifying the returned Mat may affect the original Mat.
cv::Mat convertto32f(const cv::Mat &in, bool copy)
{
	cv::Mat ret;

	auto type = in.type();
	uchar depth = type & CV_MAT_DEPTH_MASK;
	uchar chans = 1 + (type >> CV_CN_SHIFT);

	if (depth != CV_32F) {
		in.convertTo(ret, CV_MAKE_TYPE(CV_32F, depth));
	}
	else {
		if (copy) {
			ret = in.clone();
		}
		else {
			ret = in;
		}
	}

	return ret;

}

// if copy is false, this function won't necessarily make a copy of input,
// which means that modifying the returned Mat may affect the original Mat.
cv::Mat convertto64f(const cv::Mat &in, bool copy)
{
	cv::Mat ret;

	auto type = in.type();
	uchar depth = type & CV_MAT_DEPTH_MASK;
	uchar chans = 1 + (type >> CV_CN_SHIFT);

	if (depth != CV_64F) {
		in.convertTo(ret, CV_MAKE_TYPE(CV_64F, depth));
	}
	else {
		if (copy) {
			ret = in.clone();
		}
		else {
			ret = in;
		}
	}

	return ret;

}



// n: the number of Mats
// you should pass n Mat pointers (Mat*)
// all Mats passed to this function must have the same type.
cv::Mat __cdecl mat2line(int n, ...)
{
	va_list vl;
	int size = 0;
	cv::Mat ret;
	int i = 0;
	cv::Mat *mat;

	va_start(vl, n);
	for (int j = 0; j < n; j++) {
		mat = va_arg(vl, cv::Mat*);
		size += mat->rows * mat->cols;
	}
	va_end(vl);



	va_start(vl, n);

	mat = va_arg(vl, cv::Mat*);
	ret = cv::Mat::zeros(1, size, mat->type());
	int c = mat->cols;
	for (int r = 0; r < mat->rows; r++) {
		mat->row(r).copyTo(ret(cv::Rect(i, 0, c, 1)));
		i += c;
	}

	for (int j = 1; j < n; j++) {
		mat = va_arg(vl, cv::Mat*);
		if (j == 0) {
			ret = cv::Mat::zeros(1, size, mat->type());
		}
		int c = mat->cols;
		for (int r = 0; r < mat->rows; r++) {
			mat->row(r).copyTo(ret(cv::Rect(i, 0, c, 1)));
			i += c;
		}
	}

	va_end(vl);

	return ret;
}



#ifdef _DEBUG

#include <iostream>

void test_cvutils_mat32fcsv()
{
	const wchar_t *samplesfilename = L"C:\\FTP_Shared\\tmp\\results\\0001-1-cropped.tif.txt";
	const wchar_t *osamplesfilename = L"C:\\FTP_Shared\\tmp\\results\\0001-1-cropped.tifo.txt";

	cv::Mat samples;

	csv2mat32f(samplesfilename, samples);
	mat2csv(osamplesfilename, samples);
}


void test_cvutils_mat2line()
{

	cv::Mat a = cv::Mat::ones(2, 2, CV_32FC1)*2.123F;
	cv::Mat b = cv::Mat::zeros(1, 2, CV_32FC1);
	cv::Mat c = cv::Mat::ones(2, 1, CV_32FC1);

	std::cout << mat2line(3, &a, &b, &c);
}

#endif