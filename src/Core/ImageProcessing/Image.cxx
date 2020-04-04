#include "Image.hxx"
#include "TextLogging.hxx"

#include <sstream>
#include <opencv2\opencv.hpp>

namespace {
	std::ostringstream tlOss;
	TextLogging tlog;
}


EasyGrade::Image::Image() {
	imageData_ptr_ = std::make_unique<cv::Mat>();
}

EasyGrade::Image::Image(const Image& other) {
	imageData_ptr_ = std::make_unique<cv::Mat>();
	other.processableImage().copyTo(*imageData_ptr_);
}

EasyGrade::Image::~Image() = default;

// What size chunks to use when reading an unknown amount of serialized data from a stream.
static const size_t CHUNK_SIZE = 1024;

int EasyGrade::Image::read(std::istream& is) {
	int status = 0;

	//Check that input stream can be read
	if(!is.good()) {
		status = -1;
		tlOss << "Recieved input stream that cannot be read" << std::endl;
		tlog.critical(__FILE__, __LINE__, tlOss);
	}

	//Read all of the data from the input stream into a buffer
	std::vector<char> serializedData;
	if(status >= 0) {
		//Because the total size of the data in the stream may not be known, read it in chunks and then combine them together at the end once all of them are retrived

		//Small buffer to store the most recent chunk
		std::vector<char> currentChunk(CHUNK_SIZE, 0);
		//buffered stream to store and combine all of the chunks
		std::stringstream combinedChunks;
		//The total amount of data read from the input stream so far
		std::streamsize totalSize = 0;

		//Read data until the end of the stream
		while(status >= 0 && !is.eof()) {
			//Read the next chunk from the input stream
			is.read(currentChunk.data(), CHUNK_SIZE);
			std::streamsize currentChunkSize = is.gcount();

			//Append the latest chunk
			combinedChunks.write(currentChunk.data(), currentChunkSize);

			//Update the total size counter
			totalSize += currentChunkSize;

			//Check if any errors have occured
			if(is.fail()) {
				status = -1;
				tlOss << "An error occured while reading data from an input stream";
				tlog.critical(__FILE__, __LINE__, tlOss);
			}
		}

		if(status >= 0) {
			//If all data was read successfully, dump it into the buffer
			serializedData.resize(totalSize);
			combinedChunks.read(serializedData.data(), totalSize);
		}

	}

	if(status >= 0) {
		//Use OpenCV to decode the data from the input stream and store the resulting image
		cv::imdecode(serializedData, cv::IMREAD_COLOR).copyTo(*imageData_ptr_);

		if(imageData_ptr_->data) {
			tlOss << "Successfully parsed image from stream";
			tlog.debug(__FILE__, __LINE__, tlOss);
		} else {
			status = -1;
			tlOss << "Failed to parsed image";
			tlog.critical(__FILE__, __LINE__, tlOss);
		}
	}

	return status;
}

bool EasyGrade::Image::empty() const {
	return false;
}

int EasyGrade::Image::write(std::ostream& os, const std::string& format) const {
	int status = 0;

	//Setup OpenCV to output a PNG
	std::vector<int> compression_params;
	compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(3);

	//Buffer for OpenCV to write the serialized image to
	std::vector<unsigned char> serializedData;

	//Serialize the image data
	cv::imencode(format, *imageData_ptr_, serializedData, compression_params);

	//Write the serialized data to the output stream
	os.write(reinterpret_cast<char*>(serializedData.data()), serializedData.size());

	if(os.fail()) {
		status = -1;
		tlOss << "An error occured while writing an image to an output stream";
		tlog.critical(__FILE__, __LINE__, tlOss);
	}

	return status;
}

cv::Mat& EasyGrade::Image::processableImage() {
	return *imageData_ptr_;
}

const cv::Mat& EasyGrade::Image::processableImage() const {
	return *imageData_ptr_;
}

int EasyGrade::Image::getWidth() const {
	return imageData_ptr_->cols;
}

int EasyGrade::Image::getHeight() const {
	return imageData_ptr_->rows;
}

float EasyGrade::Image::toRelativeCoordinates(float absolute) const {
	return absolute / getWidth();
}

float EasyGrade::Image::toAbsoluteCoordinates(float relative) const {
	return relative * getWidth();
}
