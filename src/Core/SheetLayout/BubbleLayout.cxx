
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
	location_ = other.getLocation();
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
	return cv::Rect2f(location_[0] - location_[2], location_[1] - location_[2], 2 * location_[2], 2 * location_[2]);
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

void BubbleLayout::setLocation(const cv::Vec3f& location) {
	location_ = location;
}

void BubbleLayout::setCenterX(float x) {
	location_[0] = x;
}

void BubbleLayout::setCenterY(float y) {
	location_[1] = y;
}

void BubbleLayout::setCenter(const cv::Point2f& center) {
	location_[0] = center.x;
	location_[1] = center.y;
}

void BubbleLayout::setRadius(float r) {
	location_[2] = r;
}

const cv::Vec3f & BubbleLayout::getLocation() const {
	return location_;
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
