#include "stdafx.h"

#include "alg.h"

#include <tuple>

#include <opencv2/core.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/imgproc.hpp>

// orientation_dependent: true: sensitive to orientation of original image.
std::tuple<std::vector<cv::KeyPoint>/*keypoints*/,cv::Mat/*descriptors*/>
computesift(const cv::Mat &img, bool orientation_dependent)
{
	std::vector<cv::KeyPoint> keypoints;
	cv::Mat descriptors;

	cv::Ptr<cv::Feature2D> f2d = cv::xfeatures2d::SIFT::create();
	f2d->detect(img, keypoints, descriptors);

	if (orientation_dependent) {
		size_t keypointscount = keypoints.size();
		for (size_t i = 0; i < keypointscount; i++) {
			keypoints[i].angle = 0;
		}
	}

	f2d->compute(img, keypoints, descriptors);

	return std::make_tuple(keypoints, descriptors);
}


cv::Mat drawkeypoints(const cv::Mat &img, const std::vector<cv::KeyPoint> &keypoints)
{
	cv::Mat keypointsimg;

	cv::drawKeypoints(img, keypoints, keypointsimg, CV_RGB(255, 0, 0));

	return keypointsimg;
}


void hellinger_norm(cv::Mat &descriptors)
{
	for (int i = 0; i < descriptors.rows; i++) {
		// "row" is a REFERENCE of "descriptors"
		cv::Mat row = descriptors.row(i);
		double normvalue = cv::norm(row, CV_L1);
		if (normvalue == 0) normvalue = 1;

		row /= normvalue;
		cv::sqrt(row, row);
	}
}