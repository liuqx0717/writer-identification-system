#pragma once


#include <opencv2/core.hpp>


// ensure input Mat has zero-mean
// in: samples are stored as matrix COLUMNS
void zero_mean(cv::Mat &in);



// in: samples are stored as matrix COLUMNS
// return value: features are stored as matrix COLUMNS
cv::Mat pca(const cv::Mat &in, int k, cv::Mat &out_S);


// xRot: samples are stored as matrix COLUMNS
// return value: features are stored as matrix COLUMNS
void pca_wh(cv::Mat &xRot, const cv::Mat &S);
