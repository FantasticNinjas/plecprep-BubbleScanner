
#include <sstream>

#include "BubbleLayout.hxx"
#include "TextLogging.hxx"

namespace {
	std::ostringstream tlOss;
	TextLogging tlog;
}

EasyGrade::BubbleLayout::BubbleLayout() = default;
EasyGrade::BubbleLayout::~BubbleLayout() = default;
EasyGrade::BubbleLayout::BubbleLayout(const BubbleLayout& other) {
	answer_ = other.getAnswer();
	 location_ = other.boundingBox();
}

int EasyGrade::BubbleLayout::removeChild(SheetLayoutElement* sheetLayoutElement) {
	//Bubble layouts are always leaf elements; they never have any children. So this method should always return a positive number, indicating that the specified element is not its child
	tlOss << "removeChild was called on a bubble layout, which will never have children. This is allowed, but may not have been intended.";
	tlog.debug(__FILE__, __LINE__, tlOss);
	return 1;
}

EasyGrade::SheetLayoutElement* EasyGrade::BubbleLayout::childAt(size_t index) {
	tlOss << "childAt was called on a bubble layout, which will never have children. This is allowed, but may not have been intended.";
	tlog.debug(__FILE__, __LINE__, tlOss);
	return nullptr;
}

size_t EasyGrade::BubbleLayout::numChildren() const {
	return 0;
}

EasyGrade::Rectangle EasyGrade::BubbleLayout::boundingBox() const {
	return location_;
}

void EasyGrade::BubbleLayout::print(std::ostream& os) const {
	os << "\"" << answer_ << "\" Bubble";
}

void EasyGrade::BubbleLayout::setAnswer(const std::string & answer) {
	answer_ = answer;
}

const std::string& EasyGrade::BubbleLayout::getAnswer() const {
	return answer_;
}

EasyGrade::SheetLayoutElement* EasyGrade::BubbleLayout::getParent() const {
	return parent_;
}

void EasyGrade::BubbleLayout::setParent(SheetLayoutElement * parent) {
	parent_ = parent;
}

std::unique_ptr<EasyGrade::SheetLayoutElement> EasyGrade::BubbleLayout::clonePtr() const {
	return std::make_unique<BubbleLayout>(*this);
}

void EasyGrade::BubbleLayout::setCoordinates(const Rectangle& coordinates) {
	location_ = coordinates;
}

void EasyGrade::BubbleLayout::setLeftEdge(float x) {
	location_.setLeftEdge(x);
}

void EasyGrade::BubbleLayout::setTopEdge(float y) {
	location_.setTopEdge(y);
}

void EasyGrade::BubbleLayout::setRightEdge(float x) {
	location_.setRightEdge(x);
}

void EasyGrade::BubbleLayout::setBottomEdge(float y) {
	location_.setBottomEdge(y);
}

void EasyGrade::BubbleLayout::setCenterX(float x) {
	location_.setCenterX(x);
}

void EasyGrade::BubbleLayout::setCenterY(float y) {
	location_.setCenterY(y);
}

void EasyGrade::BubbleLayout::setWidth(float w) {
	location_.setWidthCentered(w);
}

void EasyGrade::BubbleLayout::setHeight(float h) {
	location_.setHeightCentered(h);
}

float EasyGrade::BubbleLayout::getLeftEdge() const {
	return location_.getLeftEdge();
}

float EasyGrade::BubbleLayout::getTopEdge() const {
	return location_.getTopEdge();
}

float EasyGrade::BubbleLayout::getRightEdge() const {
	return location_.getRightEdge();
}

float EasyGrade::BubbleLayout::getBottomEdge() const {
	return location_.getBottomEdge();
}

float EasyGrade::BubbleLayout::getCenterX() const {
	return location_.getCenterX();
}

float EasyGrade::BubbleLayout::getCenterY() const {
	return location_.getCenterY();
}

float EasyGrade::BubbleLayout::getWidth() const {
	return location_.getWidth();
}

float EasyGrade::BubbleLayout::getHeight() const {
	return location_.getHeight();
}
