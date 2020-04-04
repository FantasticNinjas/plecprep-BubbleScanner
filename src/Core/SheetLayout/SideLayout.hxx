#pragma once

#include <vector>
#include <string>

#include "SheetLayoutElement.hxx"
#include "GroupLayout.hxx"

namespace EasyGrade {

	class SideLayout : public SheetLayoutElement {
	public:
		SideLayout();
		SideLayout(int sideNumber);
		SideLayout(const SideLayout& other);
		~SideLayout();

		///
		/// <summary> Remove the specified child of this sheet side layout </summary>
		///
		/// <param name="sheetLayoutElement"> A pointer to the child to remove </param>
		///
		/// <returns> Zero if the child was removed successfully, negative if an error occured, positive if no error occured but the specified element was not a child of this question group </returns>
		///
		/// <note> This method deallocates the child, so any pointers to it should be discarded </note>
		///
		int removeChild(SheetLayoutElement* sheetLayoutElement);

		///
		/// <summary> Get the smallest rectangle that completely surrounds the question groups on this sheet side layout (in normalized coordinates) </summary>
		///
		Rectangle boundingBox() const;

		///
		/// <summary> Add a question group to this side layout </summary>
		/// <param name="questionGroup"> The question group to add </param>
		///
		void addGroup(const GroupLayout* questionGroup);

		///
		/// <summary> Get a pointer to a question group layout on this side layout </summary>
		/// <param name="index"> The index of the question group to get </param>
		///
		GroupLayout* groupAt(size_t index);

		///
		/// <summary> Get a pointer to a question group layout on this side layout </summary>
		/// <param name="index"> The index of the question group to get </param>
		///
		const GroupLayout* groupAt(size_t index) const;

		///
		/// <summary> Get a pointer to a question group layout on this side layout </summary>
		/// <param name="index"> The index corrisponding to the question group to get </param>
		///
		/// <note> This method does the exact same thing as SideLayout::groupAt(), except that the pointer it returns is of type SheetLayoutElement instead of GroupLayout </note>
		///
		SheetLayoutElement* childAt(size_t index);

		///
		/// <summary> Get the number of question groups on this side layout </summary>
		///
		size_t numChildren() const;

		///
		/// <summary> Get the side number for this side layout </summary>
		///
		int getSideNumber() const;

		///
		/// <summary> Get the filename for an image of the actual scan sheet that this side layout is based on </summary>
		/// <returns> A string representing the filename, or an empty string if no reference image exists </returns>
		///
		const std::string& getReferenceImageFilename() const;

		///
		/// <summary> Specify the filename for an image of the actual scan sheet that this side layout is based on </summary>
		/// <param name="filename"> The filename of the image </param>
		///
		void setReferenceImageFilename(const std::string& filename);

		///
		/// <summary> Print a short description of this sheet side layout to an output stream </summary>
		/// <param name="oss"> The output stream to print to </param>
		/// <note> This method is used by the &lt;&lt; operator and the toString() method </note>
		///
		void print(std::ostream& os) const;

		///
		/// <summary> Get a pointer to the sheet layout element that owns this side layout. There are currently no sheet layout elements that contain side layouts, so this method will always return a null pointer. </summary>
		///
		[[depricated("Because side layouts never have a parent layout.")]]
		SheetLayoutElement* getParent() const;

		std::unique_ptr<SheetLayoutElement> clonePtr() const;
	private:
		const int sideNumber_{-1};
		std::vector<std::unique_ptr<GroupLayout>> questionGroups_{};
		std::string referenceImage_{};
	};

}