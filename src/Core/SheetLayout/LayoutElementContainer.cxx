
#include <sstream>

#include "LayoutElementContainer.hxx"
#include "TextLogging.hxx"


LayoutElementContainer::LayoutElementContainer() = default;
LayoutElementContainer::~LayoutElementContainer() = default;

void LayoutElementContainer::add(const SheetLayoutElement& layoutElement) {
	layoutElements_.push_back(layoutElement.clonePtr());
} 

void LayoutElementContainer::remove(size_t index) {
	layoutElements_.erase(layoutElements_.begin() + index);
}

void LayoutElementContainer::remove(const SheetLayoutElement* element) {
	//Iterate over all of the layout elements until the desired one is found and delete it
	for(std::vector<std::unique_ptr<SheetLayoutElement> >::iterator& it = layoutElements_.begin(); it != layoutElements_.end(); it++) {
		if(it->get() == element) {
			layoutElements_.erase(it);
			//Note: it is important to break the loop after removing the element, as all iterators into a vector may become invalid when it is modified.
			break;
		}
	}
}

SheetLayoutElement* LayoutElementContainer::elementAt(size_t index) {
	SheetLayoutElement* element = nullptr;
	if(index < size()) {
		element = layoutElements_[index].get();
	}
	return element;
}

size_t LayoutElementContainer::size() const {
	return layoutElements_.size();
}

bool LayoutElementContainer::empty() const {
	return layoutElements_.empty();
}
