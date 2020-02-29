// 
// University of Massachusetts Amherst Physics Lecture Prep
//
// Project: ScantronReader
// File: SheetLayoutElement.hxx
// Author: Casey Massar (caseymassar@gmail.com)
// Description: Interface for scanner sheet layout elements, i.e. classes used to specify the location and other information about bubbles and questions (and other elements) on a scanner sheet. In general,
//              subclasses of SheetLayoutElement should store only information that a naive human can easily get by looking at the scanner sheet.
//

#pragma once

#include <opencv2/opencv.hpp>
#include <sstream>
#include <string>

class SheetLayoutElement {
public:

	///
	/// <summary> Get a pointer to a child of this sheet layout element </summary>
	///
	/// <param name="index"> The index of the child to get </param>
	///
	virtual SheetLayoutElement* childAt(size_t index) = 0;

	///
	/// <summary> Get the number of children of this sheet layout element </summary>
	///
	virtual size_t numChildren() const = 0;

	///
	/// <summary> Remove the specified child of this sheet layout element </summary>
	///
	/// <param name="sheetLayoutElement"> A pointer to the child to delete </param>
	///
	/// <returns> Zero if the child was removed successfully, negative if an error occured, positive if no error occured but the specified element was not a child of this one </returns>
	///
	/// <note> This method deallocates the child, so any pointers to it should be discarded </note>
	///
	virtual int removeChild(SheetLayoutElement* sheetLayoutElement) = 0;

	///
	/// <summary> Get the smallest rectangle that completely surrounds the sheet layout element (in normalized coordinates) </summary>
	///
	virtual cv::Rect2f boundingBox() const = 0;

	///
	/// <summary> Print a short description of the sheet layout element to an output stream </summary>
	/// <param name="oss"> The output stream to print to </param>
	/// <note> This method is used by the &lt;&lt; operator and the toString() method </note>
	///
	virtual void print(std::ostream& os) const = 0;

	///
	/// <summary> Get a short description of the sheet layout element as a string </summary>
	///
	virtual std::string toString() const final;

	///
	/// <summary> Get a pointer to the sheet layout element that owns this one, or a null pointer if it is unowned </summary>
	///
	virtual SheetLayoutElement* getParent() const = 0;

	///
	/// <summary> Create a clone of this sheet layout element </summary>
	/// <returns> A unique_ptr to the clone </returns>
	///
	virtual std::unique_ptr<SheetLayoutElement> clonePtr() const = 0;
};

std::ostream& operator<<(std::ostream& os, const SheetLayoutElement& layoutElement);

