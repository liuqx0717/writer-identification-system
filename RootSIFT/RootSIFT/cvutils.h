#pragma once

#include <opencv2/core.hpp>

bool readimgfile(const wchar_t *path, cv::Mat &out);
bool writeimgfile(const wchar_t *path, const cv::Mat &in);



bool mat2csv(const wchar_t *filename, cv::Mat &mat, int significant_digits = 8);

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