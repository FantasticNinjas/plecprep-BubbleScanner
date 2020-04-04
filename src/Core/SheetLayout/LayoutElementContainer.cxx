
#include <sstream>

#include "LayoutElementContainer.hxx"
#include "TextLogging.hxx"


EasyGrade::LayoutElementContainer::LayoutElementContainer() = default;
EasyGrade::LayoutElementContainer::~LayoutElementContainer() = default;

void EasyGrade::LayoutElementContainer::add(const SheetLayoutElement& layoutElement) {
	layoutElements_.push_back(layoutElement.clonePtr());
} 

void EasyGrade::LayoutElementContainer::remove(size_t index) {
	layoutElements_.erase(layoutElements_.begin() + index);
}

void EasyGrade::LayoutElementContainer::remove(const SheetLayoutElement* element) {
	//Iterate over all of the layout elements until the desired one is found and delete it
	for(std::vector<std::unique_ptr<SheetLayoutElement> >::iterator& it = layoutElements_.begin(); it != layoutElements_.end(); it++) {
		if(it->get() == element) {
			layoutElements_.erase(it);
			//Note: it is important to break the loop after removing the element, as all iterators into a vector may become invalid when it is modified.
			break;
		}
	}
}

EasyGrade::SheetLayoutElement* EasyGrade::LayoutElementContainer::elementAt(size_t index) {
	SheetLayoutElement* element = nullptr;
	if(index < size()) {
		element = layoutElements_[index].get();
	}
	return element;
}

size_t EasyGrade::LayoutElementContainer::size() const {
	return layoutElements_.size();
}

bool EasyGrade::LayoutElementContainer::empty() const {
	return layoutElements_.empty();
}
