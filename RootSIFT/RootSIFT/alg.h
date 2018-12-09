#pragma once


#include <vector>

#include <opencv2/core.hpp>


// orientation_dependent: true: sensitive to orientation of original image.
std::tuple<std::vector<cv::KeyPoint>/*keypoints*/, cv::Mat/*descriptors*/>
computesift(const cv::Mat &img, bool orientation_dependent);


cv::Mat drawkeypoints(const cv::Mat &img, const std::vector<cv::KeyPoint> &keypoints);


void hellinger_norm(cv::Mat &descriptors);
