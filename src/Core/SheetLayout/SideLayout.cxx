
#include <sstream>

#include "TextLogging.hxx"
#include "SideLayout.hxx"

namespace {
	std::ostringstream tlOss;
	TextLogging tlog;
}

SideLayout::SideLayout() = default;

SideLayout::SideLayout(int sideNumber) : sideNumber_(sideNumber) {}

SideLayout::SideLayout(const SideLayout& other) : sideNumber_(other.getSideNumber()) {
	referenceImage_ = other.getReferenceImageFilename();
	for(const auto& group : other.allGroups()) {
		addGroup(group);
	}
}

SideLayout::~SideLayout() = default;

int SideLayout::removeChild(SheetLayoutElement* sheetLayoutElement) {
	//Status starts at 1 and then if something gets deleted it is changed to 0
	int status = 1;
	//Iterate over all of the question groups in reverse order (so that deletions don't change the indices of children that haven't been checked yet)
	for(int i = numChildren() - 1; i >= 0; i--) {
		if(static_cast<SheetLayoutElement*>(groupAt(i)) == sheetLayoutElement) {
			//If this is the child being searched for, remove it and set the return status acordingly
			tlOss << "Removing question group " << *groupAt(i) << " from side \"" << *this << "\"";
			tlog.debug(__FILE__, __LINE__, tlOss);
			questionGroups_.erase(questionGroups_.begin() + i);
			if(status > 0) {
				status = 0;
			}
		}
	}

	return status;
}

cv::Rect2f SideLayout::boundingBox() const {
	cv::Rect2f box;
	for(const auto& group : questionGroups_) {
		box = box | group.boundingBox();
	}
	return box;
}

void SideLayout::addGroup(const GroupLayout& questionGroup) {
	questionGroups_.push_back(questionGroup);
	//Set the parent of the newly added group to this side layout
	questionGroups_.back().setParent(this);
}

GroupLayout* SideLayout::groupAt(size_t index) {
	GroupLayout* group = nullptr;
	if(index < numChildren()) {
		group = &questionGroups_[index];
	}
	return group;
}

const GroupLayout* SideLayout::groupAt(size_t index) const {
	const GroupLayout* group = nullptr;
	if(index < numChildren()) {
		group = &questionGroups_[index];
	}
	return group;
}

SheetLayoutElement* SideLayout::childAt(size_t index) {
	return static_cast<SheetLayoutElement*>(groupAt(index));
}

size_t SideLayout::numChildren() const {
	return questionGroups_.size();
}

const std::vector<GroupLayout>& SideLayout::allGroups() const {
	return questionGroups_;
}

int SideLayout::getSideNumber() const {
	return sideNumber_;
}

const std::string& SideLayout::getReferenceImageFilename() const {
	return referenceImage_;
}

void SideLayout::setReferenceImageFilename(const std::string& filename) {
	referenceImage_ = filename;
}

void SideLayout::print(std::ostream & os) const {
	if(sideNumber_ < 0) {
		os << "Unknown Side";
	} else {
		os << "Side " << sideNumber_;
	}
}

SheetLayoutElement * SideLayout::getParent() const {
	return nullptr;
}

std::unique_ptr<SheetLayoutElement> SideLayout::clonePtr() const {
	return std::make_unique<SideLayout>(*this);
}
