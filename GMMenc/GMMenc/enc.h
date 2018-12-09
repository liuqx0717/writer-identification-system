#pragma once

#include <tuple>

#include <opencv2/core.hpp>

//samples: NxD
//return value:   
//      miu:KxD     pi:1xK      sigma:DxD x K    gamma:NxK
bool gmm_em(
	cv::Mat &samples,
	unsigned int k,       // number of components
	cv::Mat &miu,
	cv::Mat &pi,
	cv::Mat &sigma,
	cv::Mat &gamma);


// return value: 
//     adapted_miu, adapted_pi, adapted_sigma 
//     (all converted to CV_32FC1)
std::tuple<cv::Mat, cv::Mat, cv::Mat> gmm_adapt(
	const cv::Mat &samples,
	float r,          // relevance factor
	const cv::Mat &miu,
	const cv::Mat &pi,
	const cv::Mat &sigma,
	const cv::Mat &gamma);

std::tuple<cv::Mat, cv::Mat, cv::Mat> gmm_normalize(
	const cv::Mat &miu,
	const cv::Mat &pi,
	const cv::Mat &sigma,
	const cv::Mat &adapted_miu,
	const cv::Mat &adapted_pi,
	const cv::Mat &adapted_sigma
);




#ifdef _DEBUG

void test_enc_gmm_em();
void test_enc_gmm_adapt();
void test_enc_gmm_normalize();

#endif