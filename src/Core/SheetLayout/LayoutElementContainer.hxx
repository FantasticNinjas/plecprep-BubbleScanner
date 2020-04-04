// 
// University of Massachusetts Amherst Physics Lecture Prep
//
// Project: ScantronReader
// File: LayoutElementContainer.hxx
// Author: Casey Massar (caseymassar@gmail.com)
// Description: Collection used to store and access an unstructured list of sheet layout elements. 
//

#pragma once

#include <vector>

#include "SheetLayoutElement.hxx"

namespace EasyGrade {

	class LayoutElementContainer {
	public:
		LayoutElementContainer();
		~LayoutElementContainer();

		///
		/// <summary> Add a sheet layout element to this container </summary>
		///
		void add(const SheetLayoutElement& layoutElement);

		///
		/// <summary> Remove a sheet layout element from this container </summary>
		/// <param name="index"> The index of the item to remove </param>
		///
		void remove(size_t index);

		///
		/// <summary> Remove a sheet layout element from this container </summary>
		/// <param name="element"> A pointer to the element to remove </param>
		///
		void remove(const SheetLayoutElement* element);

		///
		/// <summary> Get a pointer to a sheet layout element in this container </summary>
		/// <param name="index"> The index of the element to retrieve </param>
		///
		SheetLayoutElement* elementAt(size_t index);

		///
		/// <summary> Get the number of sheet layout elements present in this container </summary>
		///
		size_t size() const;

		///
		/// <summary> Check whether this container is empty </summary>
		///
		bool empty() const;

		//TODO: implement an iterator that traverses all of the elements in this container AND their children

	private:
		std::vector<std::unique_ptr<SheetLayoutElement> > layoutElements_;
	};

}