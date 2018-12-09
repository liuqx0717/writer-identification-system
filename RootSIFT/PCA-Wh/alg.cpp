#include "stdafx.h"

#include "alg.h"



#include <opencv2/core.hpp>




// ensure input Mat has zero-mean
// in: samples are stored as matrix COLUMNS
void zero_mean(cv::Mat &in)
{

	cv::Mat sum(cv::Mat::zeros(in.rows, 1, CV_32FC1));
	int c = 0;
	for (c = 0; c < in.cols; c++) {
		sum += in.col(c);
	}
	sum /= (float)c;

	for (c = 0; c < in.cols; c++) {
		in.col(c) -= sum;
	}


}


// in: samples are stored as matrix COLUMNS
// k: the number of eigenvectors to keep. pass 0 to keep all the eigenvectors.
// return value: features are stored as matrix COLUMNS
cv::Mat pca(const cv::Mat &in, int k, cv::Mat &out_S)
{
	cv::Mat x = in;
	cv::Mat xt;
	cv::transpose(x, xt);

	cv::Mat sigma = x * xt / (float)x.cols;

	cv::Mat U, S, Vt;
	cv::SVD::compute(sigma, S, U, Vt);

	cv::Mat Ut;
	cv::transpose(U, Ut);

	if (k == 0) k = x.rows;

	cv::Mat xRot = Ut(cv::Rect(0, 0, Ut.cols, k)) * x;

	out_S = S(cv::Rect(0, 0, 1, k)).clone();

	return xRot;

}


// xRot: samples are stored as matrix COLUMNS
// return value: xRot
void pca_wh(cv::Mat &xRot, const cv::Mat &S)
{
	cv::Mat tmp;
	cv::pow(S, -0.5, tmp);

	xRot = cv::Mat::diag(tmp) * xRot;



}