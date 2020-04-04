#pragma once

#include <string>
#include <vector>

#include "SheetLayoutElement.hxx"
#include "QuestionLayout.hxx"

namespace EasyGrade {

	class GroupLayout : public SheetLayoutElement {
	public:
		GroupLayout();
		GroupLayout(const GroupLayout& other);
		~GroupLayout();

		///
		/// <summary> Remove the specified child of this question group layout </summary>
		///
		/// <param name="sheetLayoutElement"> A pointer to the child to delete </param>
		///
		/// <returns> Zero if the child was removed successfully, negative if an error occured, positive if no error occured but the specified element was not a child of this question group </returns>
		///
		/// <note> This method deallocates the child, so any pointers to it should be discarded </note>
		///
		int removeChild(SheetLayoutElement* sheetLayoutElement);

		///
		/// <summary> Get the smallest rectangle that completely surrounds the questions contained in this group (in normalized coordinates) </summary>
		///
		Rectangle boundingBox() const;

		///
		/// <summary> Print a short description of this question group to an output stream </summary>
		/// <param name="oss"> The output stream to print to </param>
		/// <note> This method is used by the &lt;&lt; operator and the toString() method </note>
		///
		void print(std::ostream& os) const;

		///
		/// <summary> Get the name of this question group </summary>
		///
		const std::string& getName() const;

		///
		/// <summary> Set the name of this question group </summary>
		/// <param name="name"> The new name </summary>
		///
		void setName(const std::string& name);

		///
		/// <summary> Add a question layout to this question group </summary>
		/// <param name="question"> The question to add </param>
		///
		void addQuestion(const QuestionLayout* question);

		///
		/// <summary> Get a pointer to the i'th question in this group </summary>
		/// <param name="index"> The index of the question to get </param>
		///
		QuestionLayout* questionAt(size_t index);

		///
		/// <summary> Get a pointer to the i'th question in this group </summary>
		/// <param name="index"> The index of the question to get </param>
		///
		const QuestionLayout* questionAt(size_t index) const;

		///
		/// <summary> Get a pointer to the i'th question in this group </summary>
		/// <param name="index"> The index of the question to get </param>
		///
		/// <note> This method does the exact same thing as GroupLayout::questionAt(), except that the pointer it returns is of type SheetLayoutElement instead of QuestionLayout </note>
		///
		SheetLayoutElement* childAt(size_t index);

		///
		/// <summary> Get the number of questions currently present in this question group </summary>
		///
		size_t numChildren() const;

		///
		/// <summary> Update the internal storage of this question group to reflect changes that have been made to its question numbers. Must be called any time </summary>
		///
		void refreshQuestionNumbers();

		///
		/// <summary> Get the lowest question number present in this question group layout </summary>
		/// <returns> The lowest question number, or -1 if the question group is empty </returns>
		///
		int minQuestionNumber() const;

		///
		/// <summary> Get the highest question number present in this question gorup layout
		/// <returns> The highest question number, or -1 if the question group is empty </returns>
		///
		int maxQuestionNumber() const;

		///
		/// <summary> Get a pointer to the sheet layout element that owns this one, or a null pointer if it is unowned </summary>
		///
		SheetLayoutElement* getParent() const;

		///
		/// <summary> Inform this question group layout of which sheet layout element owns it. This must be called any time it changes owners or undefined behavoir will occur </summary>
		/// <param name="parent"> A pointer to the new parent, or a null pointer if this question is now unowned </param>
		///
		void setParent(SheetLayoutElement* parent);

		std::unique_ptr<SheetLayoutElement> clonePtr() const;
	private:
		std::string name_{};
		std::vector<std::unique_ptr<QuestionLayout>> questions_{};
		SheetLayoutElement* parent_{nullptr};
	};

}