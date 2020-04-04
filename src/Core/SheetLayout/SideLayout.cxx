
#include <sstream>

#include "TextLogging.hxx"
#include "SideLayout.hxx"

namespace {
	std::ostringstream tlOss;
	TextLogging tlog;
}

EasyGrade::SideLayout::SideLayout() = default;

EasyGrade::SideLayout::SideLayout(int sideNumber) : sideNumber_(sideNumber) {}

EasyGrade::SideLayout::SideLayout(const SideLayout& other) : sideNumber_(other.getSideNumber()) {
	referenceImage_ = other.getReferenceImageFilename();
	for(int i = 0; i < other.numChildren(); i++) {
		addGroup(other.groupAt(i));
	}
}

EasyGrade::SideLayout::~SideLayout() = default;

int EasyGrade::SideLayout::removeChild(SheetLayoutElement* sheetLayoutElement) {
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

EasyGrade::Rectangle EasyGrade::SideLayout::boundingBox() const {
	Rectangle box;
	for(const auto& group : questionGroups_) {
		box.combineUnion(group->boundingBox());
	}
	return box;
}

void EasyGrade::SideLayout::addGroup(const GroupLayout* questionGroup) {
	questionGroups_.push_back(std::make_unique<GroupLayout>(*questionGroup));
	//Set the parent of the newly added group to this side layout
	questionGroups_.back()->setParent(this);
}

EasyGrade::GroupLayout* EasyGrade::SideLayout::groupAt(size_t index) {
	GroupLayout* group = nullptr;
	if(index < numChildren()) {
		group = questionGroups_[index].get();
	}
	return group;
}

const EasyGrade::GroupLayout* EasyGrade::SideLayout::groupAt(size_t index) const {
	const GroupLayout* group = nullptr;
	if(index < numChildren()) {
		group = questionGroups_[index].get();
	}
	return group;
}

EasyGrade::SheetLayoutElement* EasyGrade::SideLayout::childAt(size_t index) {
	return static_cast<SheetLayoutElement*>(groupAt(index));
}

size_t EasyGrade::SideLayout::numChildren() const {
	return questionGroups_.size();
}

int EasyGrade::SideLayout::getSideNumber() const {
	return sideNumber_;
}

const std::string& EasyGrade::SideLayout::getReferenceImageFilename() const {
	return referenceImage_;
}

void EasyGrade::SideLayout::setReferenceImageFilename(const std::string& filename) {
	referenceImage_ = filename;
}

void EasyGrade::SideLayout::print(std::ostream & os) const {
	if(sideNumber_ < 0) {
		os << "Unknown Side";
	} else {
		os << "Side " << sideNumber_;
	}
}

EasyGrade::SheetLayoutElement* EasyGrade::SideLayout::getParent() const {
	return nullptr;
}

std::unique_ptr<EasyGrade::SheetLayoutElement> EasyGrade::SideLayout::clonePtr() const {
	return std::make_unique<SideLayout>(*this);
}
