
#include <sstream>

#include "TextLogging.hxx"
#include "QuestionLayout.hxx"

namespace {
	static std::ostringstream tlOss;
	static TextLogging tlog;
}


QuestionLayout::QuestionLayout() = default;
QuestionLayout::~QuestionLayout() = default;
QuestionLayout::QuestionLayout(const QuestionLayout& other) {
	questionNumber_ = other.getQuestionNumber();
	for(const auto& bubble : other.allBubbles()) {
		addBubble(bubble);
	}
}

int QuestionLayout::removeChild(SheetLayoutElement* sheetLayoutElement) {
	//Status starts at 1 and then if something gets deleted it is changed to 0
	int status = 1;
	//Iterate over all of the bubbles in reverse order (so that deletions don't change the indices of children that haven't been checked yet
	for(int i = numChildren() - 1; i >= 0; i--) {
		if(static_cast<SheetLayoutElement*>(bubbleAt(i)) == sheetLayoutElement) {
			tlOss << "Removing bubble " << *bubbleAt(i) << " from question " << *this << "\"";
			tlog.debug(__FILE__, __LINE__, tlOss);
			//If this is the child being searched for, remove it and set the return status acordingly
			bubbles_.erase(bubbles_.begin() + i);
			if(status > 0) {
				status = 0;
			}
		}
	}

	if(status == 1) {
		tlOss << "No bubbles on question " << *this << " match the element to be removed";
		tlog.critical(__FILE__, __LINE__, tlOss);
	}

	return status;
}

cv::Rect2f QuestionLayout::boundingBox() const {
	//The bounding box of the question is the union of the bounding boxes for all of the bubbles
	cv::Rect2f box;

	for(const auto& bubble : bubbles_) {
		box = box | bubble.boundingBox();
	}

	return box;
}

void QuestionLayout::print(std::ostream & os) const {
	if(questionNumber_ < 0) {
		os << "New Question";
	} else {
		os << "Question " << questionNumber_;
	}
}

int QuestionLayout::getQuestionNumber() const {
	return questionNumber_;
}

void QuestionLayout::setQuestionNumber(const int questionNumber) {
	questionNumber_ = questionNumber;
}

void QuestionLayout::addBubble(const BubbleLayout& bubble) {
	bubbles_.push_back(bubble);
	//Set the parent of the new bubble to this question
	bubbles_.back().setParent(this);
}

BubbleLayout* QuestionLayout::bubbleAt(size_t index) {
	BubbleLayout* bubble = nullptr;
	if(index < numChildren()) {
		bubble = &bubbles_[index];
	}
	return bubble;
}

const BubbleLayout* QuestionLayout::bubbleAt(size_t index) const {
	const BubbleLayout* bubble = nullptr;
	if(index < numChildren()) {
		bubble = &bubbles_[index];
	}
	return bubble;
}

SheetLayoutElement* QuestionLayout::childAt(size_t index) {
	return static_cast<SheetLayoutElement*>(bubbleAt(index));
}

size_t QuestionLayout::numChildren() const {
	return bubbles_.size();
}

const std::vector<BubbleLayout>& QuestionLayout::allBubbles() const {
	return bubbles_;
}

SheetLayoutElement * QuestionLayout::getParent() const {
	return parent_;
}

void QuestionLayout::setParent(SheetLayoutElement * parent) {
	parent_ = parent;
}

std::unique_ptr<SheetLayoutElement> QuestionLayout::clonePtr() const {
	return std::make_unique<QuestionLayout>(*this);
}

bool QuestionLayout::operator<(const QuestionLayout & rhs) const {
	return questionNumber_ < rhs.getQuestionNumber();
}

bool QuestionLayout::operator>(const QuestionLayout & rhs) const {
	return questionNumber_ > rhs.getQuestionNumber();
}
