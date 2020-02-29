#pragma once

#include "SheetLayoutElement.hxx"

class BubbleLayout : public SheetLayoutElement {
public:
	BubbleLayout();
	BubbleLayout(const BubbleLayout& other);
	~BubbleLayout();

	///
	/// <summary> Remove the specified child of this bubble layout. Because bubbles do not have child elements, this method will never do anything in practice. </summary>
	///
	[[depricated("Because bubbles layouts do not have children, this method will never do anything in practice.")]]
	int removeChild(SheetLayoutElement* sheetLayoutElement);

	///
	/// <summary> Get one of the children of this bubble layout. Because bubbles do not have child elements, this method will always return a null pointer in practice </summary>
	///
	[[depricated("Because bubbles layouts do not have children, this method will always return a null pointer in practice.")]]
	SheetLayoutElement* childAt(size_t index);

	///
	/// <summary> Get the number of children of this bubble element. Because bubbles do not have child elements, this method will always return 0 in practice </summary>
	///
	[[depricated("Because bubbles layouts do not have children, this method will always return 0 in practice.")]]
	size_t numChildren() const;

	///
	/// <summary> Get the smallest rectangle that completely surrounds the bubble (in normalized coordinates) </summary>
	///
	cv::Rect2f boundingBox() const;

	///
	/// <summary> Print a short description of the bubble to an output stream </summary>
	/// <param name="oss"> The output stream to print to </param>
	/// <note> This method is used by the &lt;&lt; operator and the toString() method </note>
	///
	void print(std::ostream& os) const;

	///
	/// <summary> Get the name of the answer this bubble represents (i.e. what letter is written in it) </summary>
	///
	void setAnswer(const std::string& answer);

	///
	/// <summary> Set the name of the answer this bubble represents (i.e. what letter is written in it) </summary>
	///
	const std::string& getAnswer() const;

	///
	/// <summary> Set the center position and the radius of the bubble (in normalized coordinates) </summary>
	///
	void setLocation(const cv::Vec3f& location);

	///
	/// <summary> Set the x coordinate for the center point of the bubble (in normalized coordinates) </summary>
	///
	void setCenterX(float x);

	///
	/// <summary> Set the y coordinate for the center point of the bubble (in normalized coordinates) </summary>
	///
	void setCenterY(float y);

	///
	/// <summary> Set the center point of the  bubble (in normalized coordinates) </summary>
	///
	void setCenter(const cv::Point2f& center);

	///
	/// <summary> Set the 
	void setRadius(float r);

	///
	/// <summary> Get the center position and the radius of the bubble (in normalized coordinates) </summary>
	///
	const cv::Vec3f& getLocation() const;

	///
	/// <summary> Get a pointer to the sheet layout element that owns this one, or a null pointer if it is unowned </summary>
	///
	SheetLayoutElement* getParent() const;
	
	///
	/// <summary> Inform this bubble layout of which sheet layout element owns it. This must be called any time it changes owners or undefined behavoir will occur </summary>
	/// <param name="parent"> A pointer to the new parent, or a null pointer if this bubble is now unowned </param>
	///
	void setParent(SheetLayoutElement* parent);

	std::unique_ptr<SheetLayoutElement> clonePtr() const;

private:
	std::string answer_{};
	cv::Vec3f location_{};
	SheetLayoutElement* parent_{nullptr};
};

