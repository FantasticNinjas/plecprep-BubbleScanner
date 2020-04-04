#pragma once

#include <string>
#include <vector>

#include "SideLayout.hxx"

namespace EasyGrade {

	class ScanSheetLayout {
	public:
		ScanSheetLayout();
		~ScanSheetLayout();

		class iterator {
		public:
			//Type aliases required for C++ iterator standard
			using iterator_category = std::forward_iterator_tag;
			using value_type = SheetLayoutElement;
			using difference_type = size_t;
			using pointer = SheetLayoutElement * ;
			using reference = SheetLayoutElement & ;

			iterator(ScanSheetLayout& scanSheet, int sideIndex, int groupIndex, int questionIndex, int bubbleIndex);

			ScanSheetLayout::iterator& operator++();
			ScanSheetLayout::iterator operator++(int);
			SheetLayoutElement& operator*();
			const SheetLayoutElement& operator*() const;
			SheetLayoutElement* operator->();
			const SheetLayoutElement* operator->() const;
			friend bool operator==(const ScanSheetLayout::iterator& lhs, const ScanSheetLayout::iterator& rhs);
			friend bool operator!=(const ScanSheetLayout::iterator& lhs, const ScanSheetLayout::iterator& rhs);
		private:
			//State variables used to keep track of where in the layout tree this index is
			ScanSheetLayout& scanSheet_;
			int sideIndex_;
			int groupIndex_;
			int questionIndex_;
			int bubbleIndex_;

			//Cache of the current sheet layout element, so that dereferencing this iterator is cheaper
			SheetLayoutElement* target_;
		};

		///
		/// <summary> Write an XML representation of this sheet layout to an output stream </summary>
		/// <param name="os"> The output stream to write to </param>
		/// 
		void writeXml(std::ostream& os);

		///
		/// <summary> Read an XML representation of a sheet layout from an input stream and import its layout data into this scan sheet layout </summary>
		/// <param name="is"> The input stream to from </param>
		/// <returns> Integer status code; non-negative if data was imported successfully, negative if an error occured </returns>
		///
		int readXml(std::istream& is);

		///
		/// <summary> Clear all sheet layout elements, making this layout blank </summary>
		///
		void reset();

		///
		/// <summary> Get the layout for one side of the scan sheet </summary>
		/// <param name="sideNumber"> which side to get the layout for </param>
		/// <returns> A pointer to the corrisponding side layout </returns>
		///
		SideLayout* sideLayout(int sideNumber);

		///
		/// <summary> Add a new side layout to this scanner sheet layout </summary>
		///
		void newSide();

		///
		/// <summary> Get the number of side on this sheet layout </summary>
		///
		size_t numSides();

		///
		/// <summary> Get the title of this sheet layout </summary>
		///
		const std::string& getTitle();

		///
		/// <summary> Set the title of this sheet layout </summary>
		///
		void setTitle(const std::string& title);

		ScanSheetLayout::iterator begin();
		ScanSheetLayout::iterator end();

	private:
		std::string title_{};
		std::vector<SideLayout> sheetSides_{};
	};

}