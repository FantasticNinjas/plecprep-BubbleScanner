
#include <sstream>

#include "TextLogging.hxx"
#include "QuestionLayout.hxx"

namespace {
	static std::ostringstream tlOss;
	static TextLogging tlog;
}


EasyGrade::QuestionLayout::QuestionLayout() = default;
EasyGrade::QuestionLayout::~QuestionLayout() = default;
EasyGrade::QuestionLayout::QuestionLayout(const QuestionLayout& other) {
	questionNumber_ = other.getQuestionNumber();
	for(int i = 0; i < other.numChildren(); i++) {
		addBubble(other.bubbleAt(i));
	}
}

int EasyGrade::QuestionLayout::removeChild(SheetLayoutElement* sheetLayoutElement) {
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

EasyGrade::Rectangle EasyGrade::QuestionLayout::boundingBox() const {
	//The bounding box of the question is the union of the bounding boxes for all of the bubbles
	Rectangle box;

	for(const auto& bubble_ptr : bubbles_) {
		box.combineUnion(bubble_ptr->boundingBox());
	}

	return box;
}

void EasyGrade::QuestionLayout::print(std::ostream & os) const {
	if(questionNumber_ < 0) {
		os << "New Question";
	} else {
		os << "Question " << questionNumber_;
	}
}

int EasyGrade::QuestionLayout::getQuestionNumber() const {
	return questionNumber_;
}

void EasyGrade::QuestionLayout::setQuestionNumber(const int questionNumber) {
	questionNumber_ = questionNumber;
}

void EasyGrade::QuestionLayout::addBubble(const BubbleLayout* bubble) {
	bubbles_.push_back(std::make_unique<BubbleLayout>(*bubble));
	//Set the parent of the new bubble to this question
	bubbles_.back()->setParent(this);
}

EasyGrade::BubbleLayout* EasyGrade::QuestionLayout::bubbleAt(size_t index) {
	BubbleLayout* bubble = nullptr;
	if(index < numChildren()) {
		bubble = bubbles_[index].get();
	}
	return bubble;
}

const EasyGrade::BubbleLayout* EasyGrade::QuestionLayout::bubbleAt(size_t index) const {
	const BubbleLayout* bubble = nullptr;
	if(index < numChildren()) {
		bubble = bubbles_[index].get();
	}
	return bubble;
}

EasyGrade::SheetLayoutElement* EasyGrade::QuestionLayout::childAt(size_t index) {
	return static_cast<SheetLayoutElement*>(bubbleAt(index));
}

size_t EasyGrade::QuestionLayout::numChildren() const {
	return bubbles_.size();
}

EasyGrade::SheetLayoutElement* EasyGrade::QuestionLayout::getParent() const {
	return parent_;
}

void EasyGrade::QuestionLayout::setParent(SheetLayoutElement* parent) {
	parent_ = parent;
}

std::unique_ptr<EasyGrade::SheetLayoutElement> EasyGrade::QuestionLayout::clonePtr() const {
	return std::make_unique<QuestionLayout>(*this);
}

bool EasyGrade::QuestionLayout::operator<(const QuestionLayout & rhs) const {
	return questionNumber_ < rhs.getQuestionNumber();
}

bool EasyGrade::QuestionLayout::operator>(const QuestionLayout & rhs) const {
	return questionNumber_ > rhs.getQuestionNumber();
}

bool EasyGrade::CompareQuestionPtr::operator()(const std::unique_ptr<QuestionLayout>& lhs, const std::unique_ptr<QuestionLayout>& rhs) {
	return *lhs < *rhs;
}
