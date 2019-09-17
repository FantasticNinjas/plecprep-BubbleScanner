#include "Reader.hxx"
#include "pugixml.hpp"
#include <qdebug.h>

#define BLANK_TEMPLATE_PATH "templates/blank.xml"



std::vector<unsigned char> *Reader::readTest(cv::Mat image, unsigned int length) {
	return nullptr;
}

std::vector<cv::Vec3f> Reader::findCircles(cv::Mat image) {
	std::vector<cv::Vec3f> circles;
	cv::HoughCircles(image, circles, CV_HOUGH_GRADIENT, 1, 20, 30, 20, 12, 30);
	return circles;
}

std::vector<char> *Reader::readSection(cv::Mat image, cv::Vec4f section) {
	return nullptr;
}

void Reader::checkCircles(cv::Mat &input, cv::Mat &output, std::vector<cv::Vec3f> circles) {
	for (size_t i = 0; i < circles.size(); i++)
	{
		cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		int radius = cvRound(circles[i][2]);
		// circle center  
		cv::Rect rect(center.x - radius, center.y - radius, 2 * radius, 2 * radius);
		cv::Mat submat = input(rect);
		double p = (double)cv::countNonZero(submat) / (submat.size().width*submat.size().height);	
		if (p > 0.3) {
			circle(output, center, radius, cv::Scalar(0, 0, 255), 2, 8, 0);
		}
		else {
			circle(output, center, radius, cv::Scalar(0, 255, 0), 2, 8, 0);
		}
		// circle outline
	}
}

void Reader::threshold(cv::Mat &input, cv::Mat &output, int level) {
	cv::Mat blur;
	cv::Size blurSize(5, 5);
	cv::GaussianBlur(input, blur, blurSize, 0);

	cv::Mat thresh;
	cv::adaptiveThreshold(blur, output, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 75, level);
}

void Reader::align(cv::Mat &input, cv::Mat &output) {
	// out all except bottom 
}