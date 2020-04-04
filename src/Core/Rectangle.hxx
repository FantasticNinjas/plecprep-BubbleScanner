// 
// University of Massachusetts Amherst Physics Lecture Prep
//
// Project: ScantronReader
// File: Rectangle.hxx
// Author: Casey Massar (caseymassar@gmail.com)
// Description: Simple class representing a (non-rotated) rectangle in 2D space, along with a few methods for modifying it. Note that passing these around is usually prefered over passing cv::Rect2f's around because
//              these are a little easier to work with and avoid too much dependance on OpenCV
//

#pragma once

#include <memory>

namespace EasyGrade {
	class Rectangle {
	public:
		Rectangle();
		Rectangle(float left, float top, float width, float height);
		Rectangle(const Rectangle& other);
		~Rectangle();

		///
		/// <summary> Move the left edge of this rectangle while keeping all of its other edges in the same place</summary>
		/// <param name="x"> The new coordinate for the left edge </param>
		///
		void setLeftEdge(float newLeft);

		///
		/// <summary> Move the top edge of this rectangle while keeping all of its other edges in the same place</summary>
		/// <param name="y"> The new coordinate for the top edge </param>
		///
		void setTopEdge(float newTop);

		///
		/// <summary> Move the right edge of this rectangle while keeping all of its other edges in the same place</summary>
		/// <param name="x"> The new coordinate for the right edge </param>
		///
		void setRightEdge(float newRight);

		///
		/// <summary> Move the bottom edge of this rectangle while keeping all of its other edges in the same place</summary>
		/// <param name="y"> The new coordinate for the bottom edge </param>
		///
		void setBottomEdge(float newBottom);

		///
		/// <summary> Move the rectangle horrizontally to be centered at a specified position </summary>
		/// <param name="x"> The new x coordinate of the center of the rectangle </summary>
		///
		void setCenterX(float centerX);

		///
		/// <summary> Move the rectangle vertically to be centered at a specified position </summary>
		/// <param name="x"> The new y coordinate of the center of the rectangle </summary>
		///
		void setCenterY(float centerY);

		///
		/// <summary> Adjust the width of this rectangle, keeping its center stationary </summary>
		/// <param name="w"> The new width </param>
		///
		void setWidthCentered(float newWidth);

		///
		/// <summary> Adjust the height of this rectangle, keeping the center stationary </summary>
		/// <param name="h"> The new height </param>
		///
		void setHeightCentered(float newHeight);

		///
		/// <summary> Get the coordinate for the left edge of this rectangle </summary>
		///
		float getLeftEdge() const;

		///
		/// <summary> Get the coordinate for the top edge of this rectangle </summary>
		///
		float getTopEdge() const;

		///
		/// <summary> Get the coordinate for the right edge of this rectangle </summary>
		///
		float getRightEdge() const;

		///
		/// <summary> Get the coordinate for the bottom edge of this rectangle </summary>
		///
		float getBottomEdge() const;

		///
		/// <summary> Get the X coordinate of the center of the rectangle </summary>
		///
		float getCenterX() const;

		///
		/// <summary> Get the Y coordinate of the center of the rectangle </summary>
		///
		float getCenterY() const;

		///
		/// <summary> Get the width of this rectangle </summary>
		///
		float getWidth() const;

		///
		/// <summary> Get the height of this rectangle </summary>
		///
		float getHeight() const;

		///
		/// <summary> Check whether or not this rectangle is empty (i.e. has 0 area) </summary>
		///
		bool empty() const;

		///
		/// <summary> Check whether this rectangle and another rectangle intersect </summary>
		/// <note> Two rectangles are only considered to intersect if there is an area that they both cover. An empty rectangle doesn't intersect anything, regardless of its position </note>
		///
		bool doesIntersect(const Rectangle& other) const;

		///
		/// <summary> Grow this rectangle so that it coveres the area of another rectangle (as well as the area it previously covered) </summary>
		/// <param name="other"> The rectangle to union with this one </param>
		/// <note> If this rectangle or the other rectangle (or both) are empty, their union will simply be the non-empty rectangle </note>
		///
		void combineUnion(const Rectangle& other);

		///
		/// <summary> Shrink this rectangle so that it does not cover any area that is not also covered by another rectangle (and does not contain any area it didn't previously </summary>
		/// <param name="other"> The rectangle to intersect with this one </param>
		/// <note> If this rectangle or the other rectangle (or both) are empty, their intersection will be an empty rectangle at an unspecified location </note>
		///
		void combineIntersection(const Rectangle& other);
		

	private:
		//Point (x, y) is the top left corner of the rectangle
		float left_{0.0f};
		float top_{0.0f};
		float width_{0.0f};
		float height_{0.0f};
	};

}
