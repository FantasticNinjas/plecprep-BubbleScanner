#pragma once

#include <memory>
#include <fstream>

//Forward declearation of cv::Mat so that we don't have to include opencv in the header.
namespace cv {
	class Mat;
}

namespace EasyGrade {

	class ImageOperation;

	class Image {
	public:
		///
		/// <summary> Create an empty image </summary>
		///
		Image();

		///
		/// <summary> Create a deep copy of an image </summary>
		///
		Image(const Image& other);

		~Image();

		///
		/// <summary> Read a serialized image from an input stream. Image can be in any file format supported by OpenCV </summary>
		/// <param name="is"> The stream from which to read image data </param>
		///
		int read(std::istream& is);

		///
		/// <summary> Check whether or not the image is empty (i.e. contains no pixels) </summary>
		///
		bool empty() const;

		///
		/// <summary> Serialize this image and write it to an output stream </summary>
		/// <param name="os"> The output stream to write the image data to </param>
		/// <param name="format"> How to serialize the image, specified as a file extension. I.E. ".jpg" to save as a JPEG </param>
		///
		int write(std::ostream& os, const std::string& format) const;

		///
		/// <summary> Get a (mutable) reference to the data of this image in a structure that is conducive to image processing </summary>
		///
		cv::Mat& processableImage();

		///
		/// <summary> Get a (non-mutable) reference to the data of this image in a structure that is conducive to image processing </summary>
		///
		const cv::Mat& processableImage() const;

		///
		/// <summary> Get the width of this image in pixels </summary>
		///
		int getWidth() const;

		///
		/// <summary> Get the height of this image in pixels </summary>
		///
		int getHeight() const;

		///
		/// <summary> Convert an absolute coordinate on the image to a size-relative coordinate </summary>
		/// <param name="absolute"> An absolute coordinate (units of pixels) </param>
		/// <returns> The coorisponding relative coordinate (units of image-widths) </returns>
		///
		float toRelativeCoordinates(float absolute) const;

		///
		/// <summary> Convert a size-relative coordinate on the image to an absolute coordinate </summary>
		/// <param name="absolute"> A relative coordinate (units of image-widths) </param>
		/// <returns> The coorisponding absolute coordinate (units of pixels) </returns>
		///
		float toAbsoluteCoordinates(float relative) const;

	private:
		//Image data is stored as a pointer so that cv::Mat can be fowared-declared
		std::unique_ptr<cv::Mat> imageData_ptr_;
	};

}