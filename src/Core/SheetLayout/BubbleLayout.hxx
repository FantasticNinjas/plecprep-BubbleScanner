#pragma once

#include "SheetLayoutElement.hxx"

namespace EasyGrade {

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
		Rectangle boundingBox() const;

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
		/// <summary> Set the location and dimensions of the bubble on the page (in normalized coordinates) </summary>
		/// <param name=location> A rectangle that exactly surrounds the bubble at its new coordinates </param>
		///
		void setCoordinates(const Rectangle& coordinates);

		///
		/// <summary> Move the left edge of this bubble on the page </summary>
		/// <param name="x"> The new coordinate for the left edge of this bubble (in normalized coordinates) </param>
		///
		void setLeftEdge(float x);

		///
		/// <summary> Move the top edge of this bubble on the page </summary>
		/// <param name="y"> The new coordinate for the top edge of this bubble (in normalized coordinates) </param>
		///
		void setTopEdge(float y);

		///
		/// <summary> Move the right edge of this bubble on the page </summary>
		/// <param name="x"> The new coordinate for the right edge of this bubble (in normalized coordinates) </param>
		///
		void setRightEdge(float x);

		///
		/// <summary> Move the bottom edge of this bubble on the page </summary>
		/// <param name="y"> The new coordinate for the bottom edge of this bubble (in normalized coordinates) </param>
		///
		void setBottomEdge(float y);

		///
		/// <summary> Move the bubble horrizontally to be centered at a specified position </summary>
		/// <param name="x"> The new x coordinate of the center of the bubble (in normalized coordinates </summary>
		///
		void setCenterX(float x);

		///
		/// <summary> Move the bubble vertically to be centered at a specified position </summary>
		/// <param name="x"> The new y coordinate of the center of the bubble (in normalized coordinates </summary>
		///
		void setCenterY(float y);

		///
		/// <summary> Adjust the width of this bubble, keeping the center stationary </summary>
		/// <param name="w"> The new width of the bubble </param>
		///
		void setWidth(float w);

		///
		/// <summary> Adjust the height of this bubble, keeping the center stationary </summary>
		/// <param name="h"> The new height of the bubble </param>
		///
		void setHeight(float h);

		///
		/// <summary> Get the coordinate for the left edge of this bubble </summary>
		///
		float getLeftEdge() const;

		///
		/// <summary> Get the coordinate for the top edge of this bubble </summary>
		///
		float getTopEdge() const;

		///
		/// <summary> Get the coordinate for the right edge of this bubble </summary>
		///
		float getRightEdge() const;

		///
		/// <summary> Get the coordinate for the bottom edge of this bubble </summary>
		///
		float getBottomEdge() const;

		///
		/// <summary> Get the X coordinate of the center of the bubble (in normalized coordinates </summary>
		///
		float getCenterX() const;

		///
		/// <summary> Get the Y coordinate of the center of the bubble (in normalized coordinates </summary>
		///
		float getCenterY() const;

		///
		/// <summary> Get the width of this bubble </summary>
		///
		float getWidth() const;

		///
		/// <summary> Get the height of this bubble </summary>
		///
		float getHeight() const;

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
		Rectangle location_{};
		SheetLayoutElement* parent_{nullptr};
	};

}