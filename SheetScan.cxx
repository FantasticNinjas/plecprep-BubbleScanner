
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
	sheetImage_ = other.sheetImage_;
}

SheetScan::SheetScan(const cv::Mat& sheetImage) {
	sheetImage_ = sheetImage;
}

int SheetScan::load(const std::string& filename) {
	int status = 0;

	sheetImage_ = cv::imread(filename, CV_LOAD_IMAGE_COLOR);

	if(sheetImage_.data) {
		tlOss << "Successfully oped image \"" << filename << "\"";
		tlog.debug(__FILE__, __LINE__, tlOss);
	} else {
		status = -1;
		tlOss << "Failed to open image \"" << filename << "\"";
		tlog.critical(__FILE__, __LINE__, tlOss);
	}

	if(status >= 0) {
		annotatedImage = sheetImage_.clone();
	}

	return status;
}

int SheetScan::saveAnnotated(const std::string& filename) {
	int status = 0;
	if(savePng(annotatedImage, filename) < 0) {
		tlOss << "Failed to save annotated image \"" << filename << "\"";
		tlog.critical(__FILE__, __LINE__, tlOss);
		status = -1;
	}
	return status;
}

int SheetScan::saveProcessedCache(const std::string & filename) {
	int status = 0;
	if(savePng(sheetImage_, filename) < 0) {
		tlOss << "Failed to save annotated image \"" << filename << "\"";
		tlog.critical(__FILE__, __LINE__, tlOss);
		status = -1;
	}
	return status;
}

int SheetScan::initDetection(const DetectionParams& detectionParams) {
	int status = 0;

	switch(detectionParams.getFilterType()) {
	case FilterType::THRESH_FRAC:
		status = initThreshFrac(detectionParams);
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
		status = isCircleFilledThreshFrac(circle, detectionParams);
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

int SheetScan::initThreshFrac(const DetectionParams& detectionParams) {
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
			//Check that preblue parameter is a float
			if(!detectionParams.isFloat("threshold")) {
				status = -1;
				tlOss << "Threshold property on \"" << detectionParams.getName() << "\" configuration must be a number";
				tlog.critical(__FILE__, __LINE__, tlOss);
			}

			//Check that preblur ammount is a non-negative number
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

	return status;
}

int SheetScan::isCircleFilledThreshFrac(const cv::Vec3f & circle, const DetectionParams & detectionParams) {
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

int SheetScan::savePng(const cv::Mat& image, const std::string& filename) {
	int status = 0;
	
	std::vector<int> compression_params;
	compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(9);

	try {
		cv::imwrite(filename, image, compression_params);
	}
	catch (std::runtime_error& ex) {
		tlOss << "Failed to save PNG image \"" << filename << "\": " << ex.what();
		tlog.critical(__FILE__, __LINE__, tlOss);
		status = -1;
	}
	
	return status;
}

float SheetScan::normalized(float absolute) {
	return absolute / sheetImage_.cols;
}

float SheetScan::absolute(float normalized) {
	return normalized * sheetImage_.cols;
}
