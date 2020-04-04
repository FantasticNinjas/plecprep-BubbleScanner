#pragma once


namespace EasyGrade {

	class Image;

	class ImageFilter {

		virtual int apply(const Image& src, Image& dest) = 0;
	};
}