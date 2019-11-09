
#include <sstream>

#include "SheetScan.hxx"
#include "TextLogging.hxx"

namespace {
	std::ostringstream tlOss;
	TextLogging tlog;
}

SheetScan::SheetScan() = default;
SheetScan::~SheetScan() = default;

SheetScan::SheetScan(const SheetScan& other) {
	sheetImage_ = other.sheetImage_.clone();
	annotatedImage_ = other.annotatedImage_.clone();
	processedImageCache_ = other.processedImageCache_.clone();
}

SheetScan::SheetScan(const cv::Mat& sheetImage) {
	sheetImage_ = sheetImage;
}

int SheetScan::load(const std::string& filename) {
	int status = 0;

	sheetImage_ = cv::imread(filename, CV_LOAD_IMAGE_COLOR);

	if(sheetImage_.data) {
		tlOss << "Successfully oped image \"" << filename << "\"";
		tlog.info(__FILE__, __LINE__, tlOss);
	} else {
		status = -1;
		tlOss << "Failed to open image \"" << filename << "\"";
		tlog.critical(__FILE__, __LINE__, tlOss);
	}

	if(status >= 0) {
		resetAnnotations();
	}

	return status;
}

int SheetScan::saveSheetImage(const std::string& filename) {
	int status = 0;
	if(savePng(sheetImage_, filename) < 0) {
		tlOss << "Failed to save sheet image \"" << filename << "\"";
		tlog.critical(__FILE__, __LINE__, tlOss);
		status = -1;
	}
	return status;
}

int SheetScan::saveAnnotated(const std::string& filename) {
	int status = 0;
	if(savePng(annotatedImage_, filename) < 0) {
		tlOss << "Failed to save annotated image \"" << filename << "\"";
		tlog.critical(__FILE__, __LINE__, tlOss);
		status = -1;
	}
	return status;
}

int SheetScan::saveProcessedCache(const std::string & filename) {
	int status = 0;
	if(savePng(processedImageCache_, filename) < 0) {
		tlOss << "Failed to save processed image cache \"" << filename << "\"";
		tlog.critical(__FILE__, __LINE__, tlOss);
		status = -1;
	}
	return status;
}

const cv::Mat& SheetScan::getSheetImage() {
	return sheetImage_;
}
const cv::Mat& SheetScan::getAnnotated() {
	return annotatedImage_;
}
const cv::Mat& SheetScan::getProcessedCache() {
	return processedImageCache_;
}

int SheetScan::setupAlgorithm(const DetectionParams& detectionParams) {
	int status = 0;

	switch(detectionParams.getFilterType()) {

	//Algorithms that use threshold as an initialization step
	case FilterType::THRESH_FRAC:
	case FilterType::THRESH_CONTOUR:
	case FilterType::THRESH_HCIRCLES:
		status = threshold(detectionParams);
		break;
	default:
		status = -1;
		tlOss << "Encountered unhandled filter type: " << detectionParams.getFilterType();
		tlog.warning(__FILE__, __LINE__, tlOss);
	}

	return status;
}

int SheetScan::isCircleFilled(const cv::Vec3f& circle, const DetectionParams & detectionParams) {
	int status = 0;

	switch(detectionParams.getFilterType()) {
	case FilterType::THRESH_FRAC:
		status = isCircleFilledFrac(circle, detectionParams);
		break;
	default:
		status = -1;
		tlOss << "Encountered unhandled filter type: " << detectionParams.getFilterType();
		tlog.warning(__FILE__, __LINE__, tlOss);
	}

	return status;
}

int SheetScan::findCircles(std::vector<cv::Vec3f>& circles, const DetectionParams & detectionParams) {
	int status = 0;

	switch(detectionParams.getFilterType()) {
	case FilterType::THRESH_HCIRCLES:
		status = findCirclesHough(circles, detectionParams);
		break;
	default:
		status = -1;
		tlOss << "Encountered unhandled filter type: " << detectionParams.getFilterType();
		tlog.warning(__FILE__, __LINE__, tlOss);
	}

	return status;
}

int SheetScan::alignScan(const DetectionParams& detectionParams) {
	int status = 0;

	switch(detectionParams.getFilterType()) {
	case FilterType::THRESH_CONTOUR:
		status = alignScanContour(detectionParams);
		break;
	default:
		status = -1;
		tlOss << "Encountered unhandled filter type: " << detectionParams.getFilterType();
		tlog.warning(__FILE__, __LINE__, tlOss);
	}

	return status;
}

//------------------------------//
//    THRESH_FRAC Algorithm     //
//------------------------------//

int SheetScan::threshold(const DetectionParams& detectionParams) {
	int status = 0;

	//Extract highest contrast channel (as specified by the configuration parameters) if channel property is
	//specified. Otherwise just use the whole image converted to grayscale. For example, if the pre-printed
	//circles on the sheet are green, then they will be least visible on the green channel, increasing the
	//contrast between empty circles and filled in answer bubbles.

	if(detectionParams.hasParam("channel")) {
		//Check that channel parameter is an int
		if(!detectionParams.isInt("channel")) {
			status = -1;
			tlOss << "Channel property on \"" << detectionParams.getName() << "\" configuration must be an integer";
			tlog.critical(__FILE__, __LINE__, tlOss);
		}

		//Check that channel number is a valid index
		int channelNumber;
		if(status >= 0) {
			channelNumber = detectionParams.getAsInt("channel");
			if(channelNumber >= sheetImage_.channels()) {
				status = -1;
				tlOss << "Channel property on \"" << detectionParams.getName() << "\" configuration must be between 0 and " << sheetImage_.channels();
				tlog.critical(__FILE__, __LINE__, tlOss);
			}
		}

		//Extract specified channel
		if(status >= 0) {
			tlOss << "Extracting channel number " << channelNumber << " from sheet image.";
			tlog.debug(__FILE__, __LINE__, tlOss);

			cv::extractChannel(sheetImage_, processedImageCache_, channelNumber);
		}
	} else {
		//Rather than extracting one channel, convert the image to grayscale
		tlOss << "No channel number specified; converting image to grayscale.";
		tlog.debug(__FILE__, __LINE__, tlOss);

		cv::cvtColor(sheetImage_, processedImageCache_, CV_BGR2GRAY);
	}

	//If specified in the filter config, blur image to reduce artifacts

	if(status >= 0) {
		if(detectionParams.hasParam("preblur")) {
			//Check that preblue parameter is a float
			if(!detectionParams.isFloat("preblur")) {
				status = -1;
				tlOss << "Preblur property on \"" << detectionParams.getName() << "\" configuration must be a number";
				tlog.critical(__FILE__, __LINE__, tlOss);
			}

			//Check that preblur ammount is a non-negative number
			float preblurAmmount;
			if(status >= 0) {
				preblurAmmount = detectionParams.getAsFloat("preblur");
				if(preblurAmmount < 0) {
					status = -1;
					tlOss << "Preblur property on \"" << detectionParams.getName() << "\" must be a non-negative number.";
					tlog.critical(__FILE__, __LINE__, tlOss);
				}
			}

			if(status >= 0) {
				tlOss << "Applying " << preblurAmmount << "px blur to image.";
				tlog.debug(__FILE__, __LINE__, tlOss);

				cv::Size blurSize(preblurAmmount, preblurAmmount);
				cv::GaussianBlur(processedImageCache_, processedImageCache_, blurSize, 0);
			}
		} else {
			tlOss << "No preblur specified in detection parameters, using unblurred image.";
			tlog.debug(__FILE__, __LINE__, tlOss);
		}
	}

	//Apply a threshold filter to the image (i.e. all pixels darker than the specified value become black, all
	//pixels brighter than the specified value become while).

	if(status >= 0) {
		if(detectionParams.hasParam("threshold")) {
			//Check that threshold parameter is a float
			if(!detectionParams.isFloat("threshold")) {
				status = -1;
				tlOss << "Threshold property on \"" << detectionParams.getName() << "\" configuration must be a number";
				tlog.critical(__FILE__, __LINE__, tlOss);
			}

			//Check that threshold median offset is a non-negative number
			float threshold;
			if(status >= 0) {
				threshold = detectionParams.getAsFloat("threshold");
				if(threshold < 0) {
					status = -1;
					tlOss << "Threshold property on \"" << detectionParams.getName() << "\" must be a non-negative number.";
					tlog.critical(__FILE__, __LINE__, tlOss);
				}
			}

			if(status >= 0) {
				tlOss << "Applying threshold with median offset of " << threshold << ".";
				tlog.debug(__FILE__, __LINE__, tlOss);

				cv::adaptiveThreshold(processedImageCache_, processedImageCache_, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 75, threshold);
			}
		} else {
			status = -1;
			tlOss << "No threshold parameter specified in detection parameters.";
			tlog.critical(__FILE__, __LINE__, tlOss);
		}
	}

	//If specified in the algorithm config, invert the resulting image

	if(status >= 0) {
		if(detectionParams.hasParam("invert")) {
			tlOss << "Inverting image";
			tlog.debug(__FILE__, __LINE__, tlOss);
			cv::bitwise_not(processedImageCache_, processedImageCache_);
		} else {
			tlOss << "Not inverting image.";
			tlog.debug(__FILE__, __LINE__, tlOss);
		}
	}

	return status;
}

int SheetScan::isCircleFilledFrac(const cv::Vec3f & circle, const DetectionParams & detectionParams) {
	int status = 0;

	if(!detectionParams.isFloat("fraction")) {
		status = -1;
		tlOss << "Fraction property on \"" << detectionParams.getName() << "\" configuration must be a number";
		tlog.critical(__FILE__, __LINE__, tlOss);
	}

	//Check that preblur ammount is a non-negative number
	float fraction;
	if(status >= 0) {
		fraction = detectionParams.getAsFloat("fraction");
		if(fraction < 0 || fraction > 1) {
			status = -1;
			tlOss << "Fraction property on \"" << detectionParams.getName() << "\" must be between 0 and 1.";
			tlog.critical(__FILE__, __LINE__, tlOss);
		}
	}
	 
	if(status >= 0) {
		//Convert circle position/radius to absolute coordinates

		cv::Point absoluteCenter(cvRound(absolute(circle[0])), cvRound(absolute(circle[1])));
		int absoluteRadius = cvRound(absolute(circle[2]));

		//Isolate section of image surrounding the circle to be scanned.

		cv::Rect rect(absoluteCenter.x - absoluteRadius, absoluteCenter.y - absoluteRadius, 2 * absoluteRadius, 2 * absoluteRadius);
		cv::Mat detectionRegion = processedImageCache_(rect);

		//Count number of pixels surrounding the circle. Note that the current implementation doesn't actually check a circular region, but a square one
		//surrounding the circle.

		double p = (double)cv::countNonZero(detectionRegion) / (double)(detectionRegion.cols * detectionRegion.rows);

		//Check whether the fraction of pixels in the detection region is above the level specified in the detection configuration.

		if(p >= fraction) {
			status = 1;
		} else {
			status = 0;
		}
	}

	return status;
}
//-------------------------------------//
//     CONTOUR alignment algorithm     //
//-------------------------------------//

int SheetScan::alignScanContour(const DetectionParams& detectionParams) {
	int status = 0;

	// Get the tollerance to be used when approximating contours as polygons
	if(!detectionParams.isFloat("alignment-approx-tollerance")) {
		status = -1;
		tlOss << "Poligon approximation tollerance (alignment-approx-tollerance) property on \"" << detectionParams.getName() << "\" configuration must exist and be a number";
		tlog.critical(__FILE__, __LINE__, tlOss);
	}

	float tollerance;
	if(status >= 0) {
		tollerance = detectionParams.getAsFloat("alignment-approx-tollerance");
		if(tollerance < 0) {
			status = -1;
			tlOss << "Poligon approximation tollerance (alignment-approx-tollerance) property on \"" << detectionParams.getName() << "\" must be a non-negative number.";
			tlog.critical(__FILE__, __LINE__, tlOss);
		} else {
			tlOss << "Poligon approximation tollerance set to " << tollerance;
			tlog.debug(__FILE__, __LINE__, tlOss);
		}
	}

	// Get the minimum width for a rectangle to keep
	if(!detectionParams.isFloat("alignment-min-width")) {
		status = -1;
		tlOss << "Minimum rectangle width (alignment-min-width) property on \"" << detectionParams.getName() << "\" configuration must exist and be a number";
		tlog.critical(__FILE__, __LINE__, tlOss);
	}

	float minWidth;
	if(status >= 0) {
		minWidth = detectionParams.getAsFloat("alignment-min-width");
		if(minWidth < 0) {
			status = -1;
			tlOss << "Minimum rectangle width (alignment-min-width) property on \"" << detectionParams.getName() << "\" must be a non-negative number.";
			tlog.critical(__FILE__, __LINE__, tlOss);
		} else {
			tlOss << "Minimum rectangle width set to " << minWidth;
			tlog.debug(__FILE__, __LINE__, tlOss);
		}
	}

	// Get the maximum width for a rectangle to keep
	if(!detectionParams.isFloat("alignment-max-width")) {
		status = -1;
		tlOss << "Maximum rectangle width (alignment-max-width) property on \"" << detectionParams.getName() << "\" configuration must exist and be a number";
		tlog.critical(__FILE__, __LINE__, tlOss);
	}

	float maxWidth;
	if(status >= 0) {
		maxWidth = detectionParams.getAsFloat("alignment-max-width");
		if(maxWidth < 0) {
			status = -1;
			tlOss << "Maximum rectangle width (alignment-min-width) property on \"" << detectionParams.getName() << "\" must be a non-negative number.";
			tlog.critical(__FILE__, __LINE__, tlOss);
		} else {
			tlOss << "Maximum rectangle width set to " << maxWidth;
			tlog.debug(__FILE__, __LINE__, tlOss);
		}
	}
	
	// Get the minimum height for a rectangle to keep
	if(!detectionParams.isFloat("alignment-min-height")) {
		status = -1;
		tlOss << "Minimum rectangle height (alignment-min-height) property on \"" << detectionParams.getName() << "\" configuration must exist and be a number";
		tlog.critical(__FILE__, __LINE__, tlOss);
	}

	float minHeight;
	if(status >= 0) {
		minHeight = detectionParams.getAsFloat("alignment-min-height");
		if(minHeight < 0) {
			status = -1;
			tlOss << "Minimum rectangle height (alignment-min-height) property on \"" << detectionParams.getName() << "\" must be a non-negative number.";
			tlog.critical(__FILE__, __LINE__, tlOss);
		} else {
			tlOss << "Minimum rectangle height set to " << minHeight;
			tlog.debug(__FILE__, __LINE__, tlOss);
		}
	}

	// Get the maximum height for a rectangle to keep
	if(!detectionParams.isFloat("alignment-max-height")) {
		status = -1;
		tlOss << "Maximum rectangle height (alignment-max-height) property on \"" << detectionParams.getName() << "\" configuration must exist and be a number";
		tlog.critical(__FILE__, __LINE__, tlOss);
	}

	float maxHeight;
	if(status >= 0) {
		maxHeight = detectionParams.getAsFloat("alignment-max-height");
		if(maxHeight < 0) {
			status = -1;
			tlOss << "Maximum rectangle height (alignment-min-height) property on \"" << detectionParams.getName() << "\" must be a non-negative number.";
			tlog.critical(__FILE__, __LINE__, tlOss);
		} else {
			tlOss << "Maximum rectangle height set to " << maxHeight;
			tlog.debug(__FILE__, __LINE__, tlOss);
		}
	}

	//Alignment rectangles are black. Get the minimum % filled in
	if(!detectionParams.isFloat("alignment-min-filled")) {
		status = -1;
		tlOss << "Minimum fraction filled in (alignment-min-filled) property on \"" << detectionParams.getName() << "\" configuration must exist and be a number";
		tlog.critical(__FILE__, __LINE__, tlOss);
	}

	float minFrac;
	if(status >= 0) {
		minFrac = detectionParams.getAsFloat("alignment-min-filled");
		if(minFrac < 0) {
			status = -1;
			tlOss << "Minimum fraction filled in (alignment-min-filled) property on \"" << detectionParams.getName() << "\" must be a non-negative number.";
			tlog.critical(__FILE__, __LINE__, tlOss);
		} else {
			tlOss << "Minimum fraction filled in set to " << minFrac;
			tlog.debug(__FILE__, __LINE__, tlOss);
		}
	}

	//Find the centers of all of the alignment marks
	std::vector<cv::Point> alignmentMarks;

	if(status >= 0) {
		std::vector<std::vector<cv::Point>> contours;
		cv::findContours(processedImageCache_, contours, CV_RETR_LIST, CV_CHAIN_APPROX_TC89_L1);

		for(int i = 0; i < contours.size(); i++) {
			std::vector<cv::Point> approx;
			cv::approxPolyDP(contours[i], approx, tollerance * cv::arcLength(contours[i], true), true);

			if(approx.size() != 4) {
				continue;
			}

			if(!cv::isContourConvex(approx)) {
				continue;
			}

			cv::RotatedRect boundingBox = minAreaRect(approx);

			float height = normalized(MAX(boundingBox.size.height, boundingBox.size.width));
			float width = normalized(MIN(boundingBox.size.height, boundingBox.size.width));

			//Check dimensions of contour are acceptable
			if(width < minWidth || width > maxWidth || height < minHeight || height > maxHeight) {
				continue;
			}

			if(getFilledFraction(processedImageCache_, boundingBox) < minFrac) {
				continue;
			}

			cv::drawContours(annotatedImage_, std::vector<std::vector<cv::Point>>({approx}), -1, cv::Scalar(255, 0, 255), 2);
			alignmentMarks.push_back((cv::Point)boundingBox.center);
		}

		cv::drawContours(annotatedImage_, std::vector<std::vector<cv::Point>>({alignmentMarks}), -1, cv::Scalar(255, 0, 255), 1);
	}

	cv::Point firstMark;
	cv::Point lastMark;
	if(status >= 0) {
		firstMark = alignmentMarks[0];
		lastMark = alignmentMarks[0];
		for(const cv::Point& markCenter : alignmentMarks) {
			if(markCenter.x < firstMark.x) {
				firstMark = markCenter;
			}
			if(markCenter.x > lastMark.x) {
				lastMark = markCenter;
			}
		}
	}

	//Find the offset from the leftmost mark to the rightmost mark (in order to determine how much the sheet is tilted)
	cv::Point markDelta;
	if(status >= 0) {
		markDelta = lastMark - firstMark;
	}

	if(status >= 0) {
		//Find angle that the sheet scan is tilted by
		//Note: OpenCV uses degrees for everything while the C++ standard library functions use radians
		float angleRad = std::atan2(markDelta.y, markDelta.x);
		float angleDeg = angleRad * 180.0 / 3.141592653589793238463;

		tlOss << "Sheet tilted by " << angleDeg << " degrees";
		tlog.debug(__FILE__, __LINE__, tlOss);

		//Rotate image by angle in order to reverse 

		cv::Point2f center(annotatedImage_.cols / 2., annotatedImage_.rows / 2.);
		cv::Mat rotationMatrix = cv::getRotationMatrix2D(center, angleDeg, 1.0);
		//Rotate both original sheet image and annotated image so that the change will be reflected both by subsequent image processing
		//and debug output
		cv::warpAffine(sheetImage_, sheetImage_, rotationMatrix, cv::Size(annotatedImage_.cols, annotatedImage_.rows));
		cv::warpAffine(annotatedImage_, annotatedImage_, rotationMatrix, cv::Size(annotatedImage_.cols, annotatedImage_.rows));

		//Rotate firstMark, and lastMark points so that they will be correct for later calculations
		firstMark = SheetScan::rotate(firstMark, center, -angleRad);
		lastMark = SheetScan::rotate(lastMark, center, -angleRad);
	}

	//Get the dimensions to crop the scan to. The scan will be cropped in a rectangle "around" the first alignment mark. the topOffset, bottomOffset,
	//leftOffset, and rightOffset parameters define how far the edge of the rectangle should be offset from the first alignment mark. These numbers
	//are multiplied by the distance in pixels between the first and last alignment mark. This is done so that the image will be aligned the same
	//regardless of the size of the scanning bed used
	
	//Get the bottom offset
	if(!detectionParams.isFloat("crop-offset-fraction-bottom")) {
		status = -1;
		tlOss << "Bottom crop offset (crop-offset-fraction-bottom) property on \"" << detectionParams.getName() << "\" configuration must exist and be a number";
		tlog.critical(__FILE__, __LINE__, tlOss);
	}

	float bottomOffset;
	if(status >= 0) {
		bottomOffset = detectionParams.getAsFloat("crop-offset-fraction-bottom");
		tlOss << "Bottom crop offset set to " << bottomOffset;
		tlog.debug(__FILE__, __LINE__, tlOss);
	}

	//Get the top offset
	if(!detectionParams.isFloat("crop-offset-fraction-top")) {
		status = -1;
		tlOss << "Top crop offset (crop-offset-fraction-top) property on \"" << detectionParams.getName() << "\" configuration must exist and be a number";
		tlog.critical(__FILE__, __LINE__, tlOss);
	}

	float topOffset;
	if(status >= 0) {
		topOffset = detectionParams.getAsFloat("crop-offset-fraction-top");
		tlOss << "Top crop offset set to " << topOffset;
		tlog.debug(__FILE__, __LINE__, tlOss);
	}

	//Get the left offset
	if(!detectionParams.isFloat("crop-offset-fraction-left")) {
		status = -1;
		tlOss << "Left crop offset (crop-offset-fraction-left) property on \"" << detectionParams.getName() << "\" configuration must exist and be a number";
		tlog.critical(__FILE__, __LINE__, tlOss);
	}

	float leftOffset;
	if(status >= 0) {
		leftOffset = detectionParams.getAsFloat("crop-offset-fraction-left");
		tlOss << "Left crop offset set to " << leftOffset;
		tlog.debug(__FILE__, __LINE__, tlOss);
	}

	//Get the right offset
	if(!detectionParams.isFloat("crop-offset-fraction-right")) {
		status = -1;
		tlOss << "Right crop offset (crop-offset-fraction-right) property on \"" << detectionParams.getName() << "\" configuration must exist and be a number";
		tlog.critical(__FILE__, __LINE__, tlOss);
	}

	float rightOffset;
	if(status >= 0) {
		rightOffset = detectionParams.getAsFloat("crop-offset-fraction-right");
		tlOss << "Left crop offset set to " << rightOffset;
		tlog.debug(__FILE__, __LINE__, tlOss);
	}
	
	cv::Rect cropBox;
	if(status >= 0) {
		float distance = sqrt(markDelta.x * markDelta.x + markDelta.y + markDelta.y);
		//Create rectangle around the first alignment mark with each side offset by the specified value
		cropBox.x = firstMark.x - distance * leftOffset;
		cropBox.y = firstMark.y - distance * topOffset;
		cropBox.width = distance * (leftOffset + rightOffset);
		cropBox.height = distance * (bottomOffset + topOffset);

		//Check that crop rectangle is entirely within the image. If it is not, report an error to avoid exception when cropping image.

		if((cropBox & cv::Rect(0, 0, sheetImage_.cols, sheetImage_.rows)) != cropBox) {
			status = -1;
			tlOss << "Unable to crop image, crop offset parameters exceed bounds of image.";
			tlog.critical(__FILE__, __LINE__, tlOss);
		}
	}

	if(status >= 0) {
		//Crop images to the rectangle
		sheetImage_ = sheetImage_(cropBox);
		annotatedImage_ = annotatedImage_(cropBox);
	}

	return status;
}

int SheetScan::findCirclesHough(std::vector<cv::Vec3f>& circles, const DetectionParams & detectionParams) {
	int status = 0;

	//Get parameters for Hough transform method

	//Get edge detection threshold, which determines how sensitive the edge detection algorithm is. Lower values mean more sensitivity
	if(!detectionParams.isFloat("circle-edge-detection-thresh")) {
		status = -1;
		tlOss << "Edge detection threshold (circle-edge-detection-thresh) property on \"" << detectionParams.getName() << "\" configuration must exist and be a number";
		tlog.critical(__FILE__, __LINE__, tlOss);
	}

	float edgeThreshold;
	if(status >= 0) {
		edgeThreshold = detectionParams.getAsFloat("circle-edge-detection-thresh");
		if(edgeThreshold < 0) {
			status = -1;
			tlOss << "Edge detection threshold (circle-edge-detection-thresh) property on \"" << detectionParams.getName() << "\" must be a non-negative number.";
			tlog.critical(__FILE__, __LINE__, tlOss);
		} else {
			tlOss << "Edge detection threshold set to " << edgeThreshold;
			tlog.debug(__FILE__, __LINE__, tlOss);
		}
	}

	//Get the circle accumulator threshold, which determines how circular a shape must be to be recognized as a circle. Higher values mean stricter circles
	if(!detectionParams.isFloat("circle-accumulator-thresh")) {
		status = -1;
		tlOss << "Circle accumulator threshold (circle-accumulator-thresh) property on \"" << detectionParams.getName() << "\" configuration must exist and be a number";
		tlog.critical(__FILE__, __LINE__, tlOss);
	}

	float accumThreshold;
	if(status >= 0) {
		accumThreshold = detectionParams.getAsFloat("circle-accumulator-thresh");
		if(accumThreshold < 0) {
			status = -1;
			tlOss << "Circle accumulator threshold (circle-accumulator-thresh) property on \"" << detectionParams.getName() << "\" must be a non-negative number.";
			tlog.critical(__FILE__, __LINE__, tlOss);
		} else {
			tlOss << "Circle accumulator threshold set to " << accumThreshold;
			tlog.debug(__FILE__, __LINE__, tlOss);
		}
	}

	//Get the minimum allowable distance between circle centers. This is a normalized coordinate. See SheetScan::normalize() for more details
	if(!detectionParams.isFloat("circle-min-distance")) {
		status = -1;
		tlOss << "Minimum circle distance (circle-min-distance) property on \"" << detectionParams.getName() << "\" configuration must exist and be a number";
		tlog.critical(__FILE__, __LINE__, tlOss);
	}

	int minDistanceAbsolute;
	if(status >= 0) {
		float minDistance = detectionParams.getAsFloat("circle-min-distance");
		minDistanceAbsolute = absolute(minDistance);
		if(minDistance < 0) {
			status = -1;
			tlOss << "Minimum circle distance (circle-min-distance) property on \"" << detectionParams.getName() << "\" must be a non-negative number.";
			tlog.critical(__FILE__, __LINE__, tlOss);
		} else {
			tlOss << "Minimum circle distance set to " << minDistance << " (" << minDistanceAbsolute << "px)";
			tlog.debug(__FILE__, __LINE__, tlOss);
		}
	}

	//Get the minimum allowable radius for a circle. This is a normalized coordinate.
	if(!detectionParams.isFloat("circle-min-radius")) {
		status = -1;
		tlOss << "Minimum circle radius (circle-min-radius) property on \"" << detectionParams.getName() << "\" configuration must exist and be a number";
		tlog.critical(__FILE__, __LINE__, tlOss);
	}

	int minRadiusAbsolute;
	if(status >= 0) {
		float minRadius = detectionParams.getAsFloat("circle-min-radius");
		minRadiusAbsolute = absolute(minRadius);
		if(minRadius < 0) {
			status = -1;
			tlOss << "Minimum circle radius (circle-min-radius) property on \"" << detectionParams.getName() << "\" must be a non-negative number.";
			tlog.critical(__FILE__, __LINE__, tlOss);
		} else {
			tlOss << "Minimum circle radius set to " << minRadius << " (" << minRadiusAbsolute << "px)";
			tlog.debug(__FILE__, __LINE__, tlOss);
		}
	}

	//Get the maximum allowable radius for a circle. This is a normalized coordinate.
	if(!detectionParams.isFloat("circle-max-radius")) {
		status = -1;
		tlOss << "Maximum circle radius (circle-min-radius) property on \"" << detectionParams.getName() << "\" configuration must exist and be a number";
		tlog.critical(__FILE__, __LINE__, tlOss);
	}

	int maxRadiusAbsolute;
	if(status >= 0) {
		float maxRadius = detectionParams.getAsFloat("circle-max-radius");
		maxRadiusAbsolute = absolute(maxRadius);
		if(maxRadius < 0) {
			status = -1;
			tlOss << "Maximum circle radius (circle-max-radius) property on \"" << detectionParams.getName() << "\" must be a non-negative number.";
			tlog.critical(__FILE__, __LINE__, tlOss);
		} else {
			tlOss << "Maximum circle radius set to " << maxRadius << " (" << maxRadiusAbsolute << "px)";
			tlog.debug(__FILE__, __LINE__, tlOss);
		}
	}

	//Find circles in the image
	if(status >= 0) {
		cv::HoughCircles(processedImageCache_, circles, CV_HOUGH_GRADIENT, 1, minDistanceAbsolute, edgeThreshold, accumThreshold, minRadiusAbsolute, maxRadiusAbsolute);

		//Normalize circle coordinates
		for(cv::Vec3f& circle : circles) {
			circle[0] = normalized(circle[0]);
			circle[1] = normalized(circle[1]);
			circle[2] = normalized(circle[2]);
		}
	}

	return status;
}

void SheetScan::annotateCircle(const cv::Vec3f & circle, const cv::Scalar & color, int thickness) {
	cv::Point center(absolute(circle[0]), absolute(circle[1]));
	int radius = absolute(circle[2]);
	cv::circle(annotatedImage_, center, radius, color, thickness);
}

void SheetScan::annotateCircles(const std::map<cv::Vec3f, cv::Scalar>& circles, int thickness) {
	for(auto& iter : circles) {
		annotateCircle(iter.first, iter.second, thickness);
	}
}

void SheetScan::annotateRect(const cv::Rect2f& rect, const cv::Scalar& color, int thickness) {
	cv::Rect absoluteRect(absolute(rect.x), absolute(rect.y), absolute(rect.width), absolute(rect.height));
	cv::rectangle(annotatedImage_, absoluteRect, color, thickness);
}

void SheetScan::resetAnnotations() {
	annotatedImage_ = sheetImage_.clone();
}

QPixmap SheetScan::getAnnotatedPixmap() {
	return matToPixmap(annotatedImage_);
}

QPixmap SheetScan::getOriginalPixmap() {
	return matToPixmap(sheetImage_);
}

QPixmap SheetScan::getProcessedPixmap() {
	return matToPixmap(processedImageCache_);
}

int SheetScan::savePng(const cv::Mat& image, const std::string& filename, int compressionLevel) {
	int status = 0;
	
	std::vector<int> compression_params;
	compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(compressionLevel);

	try {
		cv::imwrite(filename, image, compression_params);
	} catch (std::runtime_error& ex) {
		tlOss << "Failed to save PNG image \"" << filename << "\": " << ex.what();
		tlog.critical(__FILE__, __LINE__, tlOss);
		status = -1;
	}
	
	return status;
}

float SheetScan::normalized(float absolute) {
	return absolute / sheetImage_.cols;
}

int SheetScan::absolute(float normalized) {
	return cvRound(normalized * sheetImage_.cols);
}

cv::Point SheetScan::rotate(const cv::Point& point, const cv::Point& center, float angle) {
	//Shift axis to move center to origin
	cv::Point temp = point - center;
	//Rotate point about the origin
	temp = cv::Point(temp.x * std::cos(angle) - temp.y * std::sin(angle), temp.x * std::sin(angle) + temp.y * std::cos(angle));
	//Shift axis to move origin to center
	temp += center;
	return temp;
}

float SheetScan::getFilledFraction(const cv::Mat & image, const cv::RotatedRect & region) {
	//This function works by iterating over all of the pixels in the RotatedRect, counting how many of them are non-zero, and
	//then dividing that by the total number of pixels checked. This is performed using two LineIterators. Consider the pixels
	//inside a rotated rectangle as being arranged in a grid aligned with the rectangle. The outer for loop uses a line iterator
	//to iterate across the top of the rectangle, while the inner for loop uses a line iterator to iterate down each column of
	//the grid.

	//Because the rectangle is rotated, the actual grid of pixels may not line up exactly with the theoretical grid traversed
	//using the method above. As a result, a few isolated pixels may be skipped. For a large and fairly uniform region this
	//shouldn't significantly change the fraction of the pixels that are filled, but it means it is necessary to manually count
	//the number seeen instead of just multiplying the dimenstions of the rectangle.

	//Count the number of pixels checked
	size_t countSeen = 0;
	size_t countFilled = 0;

	//Get the points at the corners of the rectangular region
	cv::Point2f corners[4];
	region.points(corners);

	cv::Point accrossStart = corners[0];
	cv::Point accrossEnd = corners[1];
	//downOffset represents the offset between the start of a column and the end of a column of the grid described above.
	cv::Point downOffset = corners[3] - corners[0];

	//Keep track of the current position in the image.
	cv::Point currentPosition;

	//Iterate accross the top row of the region
	cv::LineIterator accrosItter = cv::LineIterator(image, accrossStart, accrossEnd, 4);
	for(int i = 0; i < accrosItter.count; ++i, ++accrosItter) {
		//Iterate down each column of the region
		cv::LineIterator downItter = cv::LineIterator(image, accrosItter.pos(), accrosItter.pos() + downOffset, 4);
		for(int j = 0; j < downItter.count; j++, ++downItter) {
			currentPosition = downItter.pos();
			//If the rectangle extands past the edge of the image, just ignore all the pixels hanging off the end
			if(currentPosition.x < image.cols && currentPosition.y < image.rows) {
				countSeen++;
				uint8_t currentPixel = image.at<uint8_t>(currentPosition);
				if(currentPixel != 0) {
					countFilled++;
				}
			}
		}
	}
	float fraction;
	if(countSeen == 0) {
		fraction = 0;
	} else {
		fraction = (float)countFilled / (float)countSeen;
	}

	return fraction;
}

QPixmap SheetScan::matToPixmap(const cv::Mat& mat) {
	//Convert the image from BGR or gray (used by OpenCV) to RGB (used by QT)
	cv::Mat rgbMat;
	if(mat.channels() == 3) {
		cv::cvtColor(mat, rgbMat, CV_BGR2RGB);
	} else {
		cv::cvtColor(mat, rgbMat, CV_GRAY2RGB);
	}
	
	if(rgbMat.data) {
		//Convert to QImage and then to QPixmap
		return QPixmap::fromImage(QImage(rgbMat.data, rgbMat.cols, rgbMat.rows, rgbMat.step, QImage::Format_RGB888));
	} else {
		return QPixmap();
	}
}
