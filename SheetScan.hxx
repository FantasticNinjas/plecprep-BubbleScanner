#pragma once

#include <string>
#include <opencv2\opencv.hpp>
#include <QPixmap>

#include "DetectionParams.hxx"


class SheetScan {
public:
	SheetScan();
	SheetScan(const SheetScan& other);
	~SheetScan();

	///
	/// <summary> Load an image from a file. </summary>
	///
	/// <param name="filename"> The filename of the image to load. </param>
	///
	/// <returns> Integer status code. Negative if an error occured, non-negative if no error occured. </returns>
	///
	int load(const std::string& filename);
	int saveSheetImage(const std::string& filename);
	int saveAnnotated(const std::string& filename);
	int saveProcessedCache(const std::string& filename);

	const cv::Mat& getSheetImage();
	const cv::Mat& getAnnotated();
	const cv::Mat& getProcessedCache();


	///
	/// <summary> Setup the image recognition algorithm used to detect wether bubbles are filled in. </summary>
	///
	/// <param name="detectionParams"> Configuration for the image recognition algorithm. </param>
	///
	/// <returns> Integer status code. Negative if an error occured, non-negative if no error occured. </returns>
	///
	int setupAlgorithm(const DetectionParams& detectionParams);

	int alignScan(const DetectionParams& detectionParams);

	///
	/// <summary> Check whether or not a circular region of the image is filled in. </summary>
	///
	/// <param name="circle"> The region to be checked. Index 0 and 1 are the X and Y coordinates of the circle center, index 2 is the radius (all using
	///					      normalized coordinates). setupAlgorithm must be called before this function </param>
	/// <param name="detectionParams"> Configuration for the image recognition algorithm. This should always be the same instance of detectionParams
	///                                as was passed to setupAlgorithm, otherwise undefined behavoir will occur. </param>
	///
	/// <returns> Positive if the circle is filled in, 0 if the circle is not filled in, negative if an error occured. </returns>
	///
	int isCircleFilled(const cv::Vec3f& circle, const DetectionParams& detectionParams);

	int findCircles(std::vector<cv::Vec3f>& circles, const DetectionParams& detectionParams);

	///
	/// <summary> Draw an empty circle on the annotated image </summary>
	///
	/// <param name="circle"> The center coordinates and radius of the circle to draw. These are considered to be normalized coordinates </param>
	/// <param name="color"> The color of the circle to draw </param>
	/// <param name="thickness"> The thickness of the circle outline, in pixels </param>
	///
	void annotateCircle(const cv::Vec3f& circle, const cv::Scalar& color, int thickness);

	void annotateCircles(const std::map<cv::Vec3f, cv::Scalar>& circles, int thickness);
	void annotateRect(const cv::Rect2f& rect, const cv::Scalar& color, int thickness);

	void resetAnnotations();

	QPixmap getAnnotatedPixmap();
	QPixmap getOriginalPixmap();
	QPixmap getProcessedPixmap();

private:

	///
	/// <summary> Apply a threshold effect to the image. All pixels whose brightness is less than a cutoff value become black, all pixels whose
	///           brightness is greater than the cutoff value become white.
	///
	/// <param name="detectionParams"> Configuration for the threshold algorithm. </param>
	///
	/// <returns> Integer status code, negative if an error occured, non-negative if no error occured. </returns>
	///
	/// <note> This function is considered part of the initialization stage of an image processing algorithm. As such, it uses sheetImage_ as its
	///        input image and processedCache_ as its output image. No member variables will be changed besides processedCache_ </note>
	///
	int threshold(const DetectionParams& detectionParams);
	int isCircleFilledFrac(const cv::Vec3f& circle, const DetectionParams& detectionParams);
	int alignScanContour(const DetectionParams& detectionParams);

	int findCirclesHough(std::vector<cv::Vec3f>& circles, const DetectionParams& detectionParams);

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
	int absolute(float normalized);

	///
	/// <summary> Rotate a 2 dimensional point around another 2 dimensional point. In other words, move the point in a circle around the center point. </summary>
	///
	/// <param name="point"> The point to rotate </param>
	/// <param name="center"> The point to rotate around </param>
	/// <param name="angle"> What angle to rotate the point (in radians)
	///
	/// <returns> The point after rotation </returns>
	///
	cv::Point rotate(const cv::Point& point, const cv::Point& center, float angle);

	///
	/// <summary> Get what fraction a rectangular region of an image is "filled in" (i.e. what fraction of the pixels in the region are non-zero). If the rectangle
	///           extends past the edge of the image the empty space will simply be ignored and will not affect the result. </summary>
	///
	/// <param name="image"> The image to check </param>
	/// <param name="region"> The region of the image, specified as a rotated rectangle </param>
	///
	/// <returns> The fraction of the region's pixels that are non-zero </returns>
	///
	/// <note> The way this function traverses the region will occasionally skip several pixels. As a result, the results of this function are somewhat imprecise.
	///        These missed pixels will never be contiguous and with moderately large regions on typical real-world images will not have a significan effect. </note>
	///
	static float getFilledFraction(const cv::Mat& image, const cv::RotatedRect& region);

	///
	/// <summary> Save an image as a PNG </summary>
	///
	/// <param name="image"> The image to save </param>
	/// <param name="filename"> Name / path for where to save the image </param>
	/// <param name="compressionLevel"> How much to compress the file. Must be an integer from 0 to 9. Will not affect image quality, but will affect file size
	///                                 an perfornamce. Smaller values will complete faster, larger values will result in smaller filea. The default value of 3
	///                                 should work well in most cases. </param>
	///
	/// <returns> Integer status code. Negative if an error occured, non-negative if no error occured. </returns>
	///
	static int savePng(const cv::Mat& image, const std::string& filename, int compressionLevel = 3);

	static QPixmap matToPixmap(const cv::Mat& mat);

	SheetScan(const cv::Mat& sheetImage);
	cv::Mat sheetImage_{};
	cv::Mat processedImageCache_{};
	cv::Mat annotatedImage_{};
};

