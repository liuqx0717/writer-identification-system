#pragma once

#include <fstream>

#include <opencv2/core.hpp>

bool csv2mat32f(const wchar_t *filename, cv::Mat &output, int channel = 1);

bool mat2csv(const wchar_t *filename, cv::Mat &mat, int significant_digits = 8);  

// if copy is false, this function won't necessarily make a copy of input,
// which means that modifying the returned Mat may affect the original Mat.
cv::Mat convertto32f(const cv::Mat &in, bool copy);

// if copy is false, this function won't necessarily make a copy of input,
// which means that modifying the returned Mat may affect the original Mat.
cv::Mat convertto64f(const cv::Mat &in, bool copy);


// n: the number of Mats
// you should pass n Mat pointers (Mat*)
// all Mats passed to this function must have the same type.
cv::Mat __cdecl mat2line(int n, ...);


template<typename T>
bool __matf2csv(const wchar_t *filename, cv::Mat &mat, int significant_digits = 8)
{
	const unsigned int numbuffersize = 128;
	char numstr[numbuffersize];

	std::fstream file(filename, std::fstream::out | std::fstream::trunc);
	if (!file) return false;
	for (int r = 0; r < mat.rows; r++) {
		const T *row = mat.ptr<T>(r);

		int c = 0;
		T num = 0;

		//with comma
		int totalcols = mat.cols*mat.channels();
		for (c = 0; c < totalcols - 1; c++) {
			num = row[c];
			//returns the number of bytes stored in buffer, not counting the terminating null character
			int strlen = sprintf(numstr, "%.*g, ", significant_digits, num);
			file.write(numstr, strlen);
		}

		//without comma
		num = row[c];
		int strlen = sprintf(numstr, "%.*g\n", significant_digits, num);
		file.write(numstr, strlen);
	}


	return true;

}











#ifdef _DEBUG

void test_cvutils_mat32fcsv();
void test_cvutils_mat2line();

#endif