
#include <sstream>

#include "BubbleLayout.hxx"
#include "TextLogging.hxx"

namespace {
	std::ostringstream tlOss;
	TextLogging tlog;
}

BubbleLayout::BubbleLayout() = default;
BubbleLayout::~BubbleLayout() = default;
BubbleLayout::BubbleLayout(const BubbleLayout& other) {
	answer_ = other.getAnswer();
	 coordinates_ = other.boundingBox();
}

int BubbleLayout::removeChild(SheetLayoutElement* sheetLayoutElement) {
	//Bubble layouts are always leaf elements; they never have any children. So this method should always return a positive number, indicating that the specified element is not its child
	tlOss << "removeChild was called on a bubble layout, which will never have children. This is allowed, but may not have been intended.";
	tlog.debug(__FILE__, __LINE__, tlOss);
	return 1;
}

SheetLayoutElement * BubbleLayout::childAt(size_t index) {
	tlOss << "childAt was called on a bubble layout, which will never have children. This is allowed, but may not have been intended.";
	tlog.debug(__FILE__, __LINE__, tlOss);
	return nullptr;
}

size_t BubbleLayout::numChildren() const {
	return 0;
}

cv::Rect2f BubbleLayout::boundingBox() const {
	return coordinates_;
}

void BubbleLayout::print(std::ostream& os) const {
	os << "\"" << answer_ << "\" Bubble";
}

void BubbleLayout::setAnswer(const std::string & answer) {
	answer_ = answer;
}

const std::string & BubbleLayout::getAnswer() const {
	return answer_;
}

SheetLayoutElement* BubbleLayout::getParent() const {
	return parent_;
}

void BubbleLayout::setParent(SheetLayoutElement * parent) {
	parent_ = parent;
}

std::unique_ptr<SheetLayoutElement> BubbleLayout::clonePtr() const {
	return std::make_unique<BubbleLayout>(*this);
}

void BubbleLayout::setCoordinates(const cv::Rect2f& coordinates) {
	coordinates_ = coordinates;
}

void BubbleLayout::setLeftEdge(float x) {
	//Adjust the width of the rectangle to keep the right edge the same. Using the setWidth function because it checks for negative widths
	setWidth(coordinates_.width - (x - coordinates_.x));
	coordinates_.x = x;
}

void BubbleLayout::setTopEdge(float y) {
	//Adjust the height of the rectangle to keep the bottom edge the same. Using the setHeight function because it checks for negative heights
	setHeight(coordinates_.height - (y - coordinates_.y));
	coordinates_.y = y;
}

void BubbleLayout::setRightEdge(float x) {
	//If the new right edge will be left of the current left edge, move over the left edge to accomodate it.
	if(x < coordinates_.x) {
		coordinates_.x = x;
		coordinates_.width = 0;
	} else {
		coordinates_.width = x - coordinates_.x;
	}
}

void BubbleLayout::setBottomEdge(float y) {
	//If the new bottom edge will be above the current top edge, move up the top edge to accomodate it.
	if(y < coordinates_.y) {
		coordinates_.y = y;
		coordinates_.height = 0;
	} else {
		coordinates_.height = y - coordinates_.y;
	}
}

void BubbleLayout::setCenterX(float x) {
	coordinates_.x = x - coordinates_.width / 2;
}

void BubbleLayout::setCenterY(float y) {
	coordinates_.y = y - coordinates_.height / 2;
}

void BubbleLayout::setWidth(float w) {
	if(w < 0) {
		tlOss << "Negative widths for bubbles are not allowed, using 0 instead.";
		tlog.debug(__FILE__, __LINE__, tlOss);
		w = 0;
	}
	//Change the width of the rectangle and then move its center back to where it used to be.
	float centerX = getCenterX();
	coordinates_.width = w;
	setCenterX(centerX);
}

void BubbleLayout::setHeight(float h) {
	if(h < 0) {
		tlOss << "Negative heights for bubbles are not allowed, using 0 instead.";
		tlog.debug(__FILE__, __LINE__, tlOss);
		h = 0;
	}
	//Change the height of the rectangle and then move its center back to where it used to be.
	float centerY = getCenterY();
	coordinates_.height = h;
	setCenterY(centerY);
}

float BubbleLayout::getLeftEdge() const {
	return coordinates_.x;
}

float BubbleLayout::getTopEdge() const {
	return coordinates_.y;
}

float BubbleLayout::getRightEdge() const {
	return coordinates_.x + coordinates_.width;
}

float BubbleLayout::getBottomEdge() const {
	return coordinates_.y + coordinates_.height;
}

float BubbleLayout::getCenterX() const {
	return coordinates_.x + coordinates_.width / 2;
}

float BubbleLayout::getCenterY() const {
	return coordinates_.y + coordinates_.height / 2;
}

float BubbleLayout::getWidth() const {
	return coordinates_.width;
}

float BubbleLayout::getHeight() const {
	return coordinates_.height;
}
