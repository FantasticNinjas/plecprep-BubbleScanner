
#include <algorithm>

#include "Rectangle.hxx"


EasyGrade::Rectangle::Rectangle() = default;

EasyGrade::Rectangle::Rectangle(float x, float y, float width, float height) : left_(x), top_(y), width_(width), height_(height) {
	//Make sure width and height aren't negative
	if(width_ < 0.0f) {
		width_ = 0;
	}

	if(height_ < 0.0f) {
		width = 0.0f;
	}
}

EasyGrade::Rectangle::Rectangle(const Rectangle& other) : left_(other.getLeftEdge()), top_(other.getTopEdge()), width_(other.getWidth()), height_(other.getHeight()) {}

EasyGrade::Rectangle::~Rectangle() = default;

void EasyGrade::Rectangle::setLeftEdge(float newLeft) {
	//Change the width of the rectangle to keep the right edge where it is
	width_ -= (newLeft - left_);
	left_ = newLeft;
	//Negative widths are not allowed, so if width has become negative than make it 0.
	if(width_ < 0.0f) {
		width_ = 0.0f;
	}
}

void EasyGrade::Rectangle::setTopEdge(float newTop) {
	//Change the height of the rectangle to keep the bottom edge where it is
	height_ -= (newTop - top_);
	top_ = newTop;
	//Negative heights are not allowed, so if height has become negative then make it 0.
	if(height_ < 0.0f) {
		height_ = 0.0f;
	}
}

void EasyGrade::Rectangle::setRightEdge(float newRight) {
	if(newRight > left_) {
		//If the new right edge of the rectangle is still to the right of the left edge, just change the width to move the right edge.
		width_ = newRight - left_;
	} else {
		//If the new right edge is to the left of the left edge, drag the left edge along with the right edge
		left_ = newRight;
		width_ = 0.0f;
	}
}

void EasyGrade::Rectangle::setBottomEdge(float newBottom) {
	if(newBottom > top_) {
		//If the new bottom edge of the rectangle is still below the top edge, just change the height to move the bottom edge.
		height_ = newBottom - top_;
	} else {
		//If the new bottom edge is above the top edge, drag the top edge along with the bottom edge
		top_ = newBottom;
		width_ = 0.0f;
	}
}

void EasyGrade::Rectangle::setCenterX(float centerX) {
	left_ = centerX - width_ / 2.0f;
}

void EasyGrade::Rectangle::setCenterY(float centerY) {
	top_ = centerY - height_ / 2.0f;
}

void EasyGrade::Rectangle::setWidthCentered(float newWidth) {
	//Keep track of the old center position so we can move the rectangle back there afte we shrink it.
	float centerX = getCenterX();
	//Change the width of the rectangle. This will keep the left edge still and move the right edge
	//If a negative width is given, just set the width to 0
	if(newWidth > 0.0f) {
		width_ = newWidth;
	} else {
		width_ = 0.0f;
	}

	//Move the rectangle back to its previous center position
	setCenterX(centerX);
}

void EasyGrade::Rectangle::setHeightCentered(float newHeight) {
	//Keep track of the old center position so we can move the rectangle back there afte we shrink it.
	float centerY = getCenterY();
	//Change the height of the rectangle. This will keep the top edge still and move the bottom edge
	//If a negative height is given, just set the height to 0
	if(newHeight > 0.0f) {
		height_ = newHeight;
	} else {
		height_ = 0.0f;
	}

	//Move the rectangle back to its previous center position
	setCenterY(centerY);
}

float EasyGrade::Rectangle::getLeftEdge() const {
	return left_;
}

float EasyGrade::Rectangle::getTopEdge() const {
	return top_;
}

float EasyGrade::Rectangle::getRightEdge() const {
	return left_ + width_;
}

float EasyGrade::Rectangle::getBottomEdge() const {
	return top_ + height_;
}

float EasyGrade::Rectangle::getCenterX() const {
	return left_ + width_ / 2.0f;
}

float EasyGrade::Rectangle::getCenterY() const {
	return top_ + height_ / 2.0f;
}

float EasyGrade::Rectangle::getWidth() const {
	return width_;
}

float EasyGrade::Rectangle::getHeight() const {
	return height_;
}

bool EasyGrade::Rectangle::empty() const {
	return width_ <= 0.0f || height_ <= 0.0f;
}

bool EasyGrade::Rectangle::doesIntersect(const Rectangle& other) const {
	float otherLeftEdge = other.getLeftEdge();
	float otherRightEdge = other.getRightEdge();
	float otherTopEdge = other.getTopEdge();
	float otherBottomEdge = other.getBottomEdge();

	float leftEdge = getLeftEdge();
	float rightEdge = getRightEdge();
	float topEdge = getTopEdge();
	float bottomEdge = getBottomEdge();

	if(!empty() && !other.empty() && //Rectangles must be non-empty to be intersecting
	   otherLeftEdge < rightEdge && //Other rectangle must not be entirely to the right of this one
	   otherRightEdge > leftEdge && //Other rectangle must not be entirely to the left of this one
	   otherTopEdge < bottomEdge && //Other rectangle must not be entirely below this one
	   otherBottomEdge > topEdge) {
		int i = 0;
		i++;
	}

	return !empty() && !other.empty() && //Rectangles must be non-empty to be intersecting
		other.getLeftEdge() < getRightEdge() && //Other rectangle must not be entirely to the right of this one
		other.getRightEdge() > getLeftEdge() && //Other rectangle must not be entirely to the left of this one
		other.getTopEdge() < getBottomEdge() && //Other rectangle must not be entirely below this one
		other.getBottomEdge() > getTopEdge(); //Other rectangle must not be entirely above this one
}

void EasyGrade::Rectangle::combineUnion(const Rectangle& other) {
	if(empty()) {
		//If this rectangle is empty, the resulting rectangle should be the other rectangle
		setLeftEdge(other.getLeftEdge());
		setTopEdge(other.getTopEdge());
		setRightEdge(other.getRightEdge());
		setBottomEdge(other.getBottomEdge());
	} else if(!other.empty()) {
		//If neither rectangle is empty, then for each edge of this rectangle, compare it to the corrisponding edge of the other rectangle and choose whichever one makes the rectangle bigger
		setLeftEdge(std::min(getLeftEdge(), other.getLeftEdge()));
		setTopEdge(std::min(getTopEdge(), other.getTopEdge()));
		setRightEdge(std::max(getRightEdge(), other.getRightEdge()));
		setBottomEdge(std::max(getBottomEdge(), other.getBottomEdge()));
	}
	//If the other rectangle is empty, we don't have to do anything; their union is just this rectangle.
}

void EasyGrade::Rectangle::combineIntersection(const Rectangle& other) {
	if(other.empty()) {
		//If the other rectangle is empty, their intersection is empty
		setLeftEdge(0.0f);
		setTopEdge(0.0f);
		setRightEdge(0.0f);
		setBottomEdge(0.0f);
	} else if(!empty()) {
		//If neither rectangle is empty, then for each edge of this rectangle, compare it to the corrisponding edge of the other rectangle and choose whichever one makes the rectangle smaller
		setLeftEdge(std::max(getLeftEdge(), other.getLeftEdge()));
		setTopEdge(std::max(getTopEdge(), other.getTopEdge()));
		setRightEdge(std::min(getRightEdge(), other.getRightEdge()));
		setBottomEdge(std::min(getBottomEdge(), other.getBottomEdge()));
	}
	//If this rectangle is empty, we don't have to do anything regardless of the other rectangle; their union is also empty.
}
