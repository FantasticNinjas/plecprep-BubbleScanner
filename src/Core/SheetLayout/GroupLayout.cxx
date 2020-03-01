#include "TextLogging.hxx"
#include "GroupLayout.hxx"

namespace {
	static std::ostringstream tlOss;
	static TextLogging tlog;
}

GroupLayout::GroupLayout() = default;
GroupLayout::~GroupLayout() = default;

GroupLayout::GroupLayout(const GroupLayout& other) {
	name_ = other.getName();
	for(int i = 0; i < other.numChildren(); i++) {
		addQuestion(other.questionAt(i));
	}
}

int GroupLayout::removeChild(SheetLayoutElement * sheetLayoutElement) {
	//Status starts at 1 and then if something gets deleted it is changed to 0
	int status = 1;
	//Iterate over all of the questions in reverse order (so that deletions don't change the indices of children that haven't been checked yet
	for(int i = numChildren() - 1; i >= 0; i--) {
		if(static_cast<SheetLayoutElement*>(questionAt(i)) == sheetLayoutElement) {
			tlOss << "Removing question " << *questionAt(i) << " from question group " << *this << "\"";
			tlog.debug(__FILE__, __LINE__, tlOss);
			//If this is the child being searched for, remove it and set the return status acordingly
			questions_.erase(questions_.begin() + i);
			if(status > 0) {
				status = 0;
			}
		}
	}

	return status;
}

cv::Rect2f GroupLayout::boundingBox() const {
	cv::Rect2f box;
	for(const auto& question_ptr : questions_) {
		box = box | question_ptr->boundingBox();
	}
	return box;
}

void GroupLayout::print(std::ostream & os) const {
	if(name_.empty()) {
		os << "New Question Group";
	} else {
		os << name_;
	}
}

const std::string & GroupLayout::getName() const {
	return name_;
}

void GroupLayout::setName(const std::string & name) {
	name_ = name;
}

void GroupLayout::addQuestion(const QuestionLayout* question) {
	//Search through the list of questions to find the position where this one belongs (or until the end of the list is found)
	size_t i = 0;
	while(i < numChildren() && *question < *questions_[i]) {
		i++;
	}
	//Insert the new question
	questions_.insert(questions_.begin() + i, std::make_unique<QuestionLayout>(*question));

	// Set the parent of the new question
	questionAt(i)->setParent(this);

}

QuestionLayout* GroupLayout::questionAt(size_t index) {
	QuestionLayout* question = nullptr;
	if(index < numChildren()) {
		question = questions_[index].get();
	}
	return question;
}

const QuestionLayout* GroupLayout::questionAt(size_t index) const {
	const QuestionLayout* question = nullptr;
	if(index < numChildren()) {
		question = questions_[index].get();
	}
	return question;
}

SheetLayoutElement* GroupLayout::childAt(size_t index) {
	return static_cast<SheetLayoutElement*>(questionAt(index));
}

size_t GroupLayout::numChildren() const {
	return questions_.size();
}

void GroupLayout::refreshQuestionNumbers() {
	std::sort(questions_.begin(), questions_.end());
}

int GroupLayout::minQuestionNumber() const {
	int min = -1;
	if(numChildren() > 0) {
		min = questions_[0]->getQuestionNumber();
	}
	return min;
}

int GroupLayout::maxQuestionNumber() const {
	int max = -1;
	if(numChildren() > 0) {
		max = questions_[numChildren() - 1]->getQuestionNumber();
	}
	return max;
}

SheetLayoutElement* GroupLayout::getParent() const {
	return parent_;
}

void GroupLayout::setParent(SheetLayoutElement* parent) {
	parent_ = parent;
}

std::unique_ptr<SheetLayoutElement> GroupLayout::clonePtr() const {
	return std::make_unique<GroupLayout>(*this);
}
