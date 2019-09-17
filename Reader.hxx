#pragma once

#include <opencv2/opencv.hpp>


class Reader {
public:
	static std::vector<unsigned char> *readTest(cv::Mat image, unsigned int length);
	static std::vector<cv::Vec3f> findCircles(cv::Mat image);
	static void threshold(cv::Mat &input, cv::Mat &output, int level);
	static void checkCircles(cv::Mat &input, cv::Mat &output, std::vector<cv::Vec3f> circles);
	static void align(cv::Mat &input, cv::Mat &output);
private:
	static std::vector<char> *readSection(cv::Mat image, cv::Vec4f section);
};

