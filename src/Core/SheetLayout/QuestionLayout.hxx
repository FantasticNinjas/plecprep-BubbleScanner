#pragma once

#include "SheetLayoutElement.hxx"
#include "BubbleLayout.hxx"

class QuestionLayout : public SheetLayoutElement {
public:
	QuestionLayout();
	QuestionLayout(const QuestionLayout& other);
	~QuestionLayout();

	///
	/// <summary> Remove the specified child of this question </summary>
	///
	/// <param name="sheetLayoutElement"> A pointer to the child to delete </param>
	///
	/// <returns> Zero if the child was removed successfully, negative if an error occured, positive if no error occured but the specified element was not a child of this one </returns>
	///
	/// <note> This method deallocates the child, so any pointers to it should be discarded </note>
	///
	int removeChild(SheetLayoutElement* sheetLayoutElement);

	///
	/// <summary> Get the smallest rectangle that completely surrounds all of the question's bubbles (in normalized coordinates) </summary>
	///
	cv::Rect2f boundingBox() const;

	///
	/// <summary> Print a short description of the question to an output stream </summary>
	/// <param name="oss"> The output stream to print to </param>
	/// <note> This method is used by the &lt;&lt; operator and the toString() method </note>
	///
	void print(std::ostream& os) const;

	///
	/// <summary> Get the question number represented by this question layout </summary>
	///
	int getQuestionNumber() const;

	///
	/// <summary> Set the question number for this question layolut </summary>
	/// <param name="questionNumber"> The new question number </param>
	///
	void setQuestionNumber(const int questionNumber);

	///
	/// <summary> Add a bubble to this question layout </summary>
	/// <param name="bubble"> The bubble to add </param>
	///
	void addBubble(const BubbleLayout* bubble);

	///
	/// <summary> Get a pointer to one of the bubble layouts owned by this question by its index </summary>
	/// <param name="index"> The index of the bubble to get. In general, bubbles are not stored in any particular order </param>
	/// <returns> The requested bubble </returns>
	///
	BubbleLayout* bubbleAt(size_t index);

	///
	/// <summary> Get a pointer to one of the bubble layouts on this question by its index </summary>
	/// <param name="index"> The index of the bubble to get. In general, bubbles are not stored in any particular order </param>
	/// <returns> The requested bubble </returns>
	///
	const BubbleLayout* bubbleAt(size_t index) const;

	///
	/// <summary> Get a pointer to one of the bubble layouts on this question by its index </summary>
	/// <param name="index"> The index of the bubble to get. In general, bubbles are not stored in any particular order </param>
	/// <returns> The requested bubble </returns>
	///
	/// <note> This method does the exact same thing as QuestionLayout::bubbleAt(), except that the pointer it returns is of type SheetLayoutElement instead of BubbleLayout </note>
	///
	SheetLayoutElement* childAt(size_t index);

	/// 
	/// <summary> Get the number of bubble layouts owned by this question </summary>
	///
	size_t numChildren() const;

	///
	/// <summary> Get a pointer to the sheet layout element that owns this one, or a null pointer if it is unowned </summary>
	///
	SheetLayoutElement* getParent() const;

	///
	/// <summary> Inform this question layout of which sheet layout element owns it. This must be called any time it changes owners or undefined behavoir will occur </summary>
	/// <param name="parent"> A pointer to the new parent, or a null pointer if this question is now unowned </param>
	///
	void setParent(SheetLayoutElement* parent);

	std::unique_ptr<SheetLayoutElement> clonePtr() const;

	///
	/// <summary> Checks whether one question comes before another in terms of question number </summary>
	/// <returns> true if the question number of the left hand operand is strictly less than the question number of the right hand operand, otherwise false </returns>
	///
	bool operator<(const QuestionLayout& rhs) const;
	bool operator>(const QuestionLayout& rhs) const;
private:
	int questionNumber_{-1};
	std::vector<std::unique_ptr<BubbleLayout>> bubbles_{};
	SheetLayoutElement* parent_{nullptr};
};

//Predicate used to sort unique_ptrs to QuestionLayouts
struct CompareQuestionPtr {
	bool operator()(const std::unique_ptr<QuestionLayout>& lhs, const std::unique_ptr<QuestionLayout>& rhs);
};

