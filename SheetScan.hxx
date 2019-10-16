#pragma once

#include <string>
#include <opencv2\opencv.hpp>

#include "DetectionParams.hxx"


class SheetScan {
public:
	SheetScan();
	SheetScan(const SheetScan& other);
	~SheetScan();

	int load(const std::string& filename);
	int saveAnnotated(const std::string& filename);
	int saveProcessedCache(const std::string& filename);

	///
	/// <summary> Setup the image recognition algorithm used to detect wether bubbles are filled in. </summary>
	///
	/// <param name="detectionParams"> Configuration for the image recognition algorithm. </param>
	///
	/// <returns> Integer status code. Negative if an error occured, non-negative if no error occured. </returns>
	///
	int initDetection(const DetectionParams& detectionParams);

	///
	/// <summary> Check whether or not a circular region of the image is filled in. </summary>
	///
	/// <param name="circle"> The region to be checked. Index 0 and 1 are the X and Y coordinates of the circle center, index 2 is the radius (all using
	///					      normalized coordinates) </param>
	/// <param name="detectionParams"> Configuration for the image recognition algorithm. This should always be the same instance of detectionParams
	///                                as was passed to initDetection, otherwise undefined behavoir will occur. </param>
	///
	/// <returns> Positive if the circle is filled in, 0 if the circle is not filled in, negative if an error occured. </returns>
	///
	int isCircleFilled(const cv::Vec3f& circle, const DetectionParams& detectionParams);
private:

	int initThreshFrac(const DetectionParams& detectionParams);

	int isCircleFilledThreshFrac(const cv::Vec3f& circle, const DetectionParams& detectionParams);

	///
	/// <summary> Convert an absolute coordinate on an image to a "normalized" coordinate system that does not depend on image resolution. </summary>
	///
	/// <param name="absolute"> The absolute coordinate (with units of pixels) to be converted </returns>
	///
	/// <returns> The corrisponding normalized coordinate. </returns>
	///
	/// <note> This function does not use the coordinate system that is typically meant by "normalized" coordinates. The coordinate systems used by this function
	///        keeps 0 in the corner, makes the highest x coordinate 1, and keeps pixels square (i.e. the highest y coordinate depends on the aspect ratio.</note>
	///
	float normalized(float absolute);

	///
	/// <summary> Convert a "normalized" coordinate on an image to an absolute coordinate. </summary>
	///
	/// <param name="normalized"> The normalized coordinate (in the coordinate system use by SheetScan::normalized()) to be converted.</param>
	///
	/// <returns> Thie corrisponding absolute coordinate </returns>
	///
	/// <note> This function does not use the coordinate system that is typically meant by "normalized" coordinates. The coordinate systems used by this function
	///        keeps 0 in the corner, makes the highest x coordinate 1, and keeps pixels square (i.e. the highest y coordinate depends on the aspect ratio.</note>
	///
	float absolute(float normalized);

	static int savePng(const cv::Mat& image, const std::string& filename);

	SheetScan(const cv::Mat& sheetImage);
	cv::Mat sheetImage_{};
	cv::Mat processedImageCache_{};
	cv::Mat annotatedImage{};
};

