
#include <sstream>

#include "TextLogging.hxx"
#include "SheetLayoutTreeBuilder.hxx"
#include "UnassignedLayoutElements.hxx"

namespace {
	std::ostringstream tlOss;
	TextLogging tlog;
}


UnassignedLayoutElements::UnassignedLayoutElements() = default;
UnassignedLayoutElements::~UnassignedLayoutElements() = default;

void UnassignedLayoutElements::add(const QuestionGroupLayout & questionGroupLayout) {
	questionGroupLayouts_.push_back(questionGroupLayout);
}

void UnassignedLayoutElements::add(const QuestionLayout & questionLayout) {
	questionLayouts_.push_back(questionLayout);
}

void UnassignedLayoutElements::add(const BubbleLayout & bubbleLayout) {
	bubbleLayouts_.push_back(bubbleLayout);
}

int UnassignedLayoutElements::numQuestionGroups() const {
	return questionGroupLayouts_.size();
}

int UnassignedLayoutElements::numQuestions() const {
	return questionLayouts_.size();
}

int UnassignedLayoutElements::numBubbles() const {
	return bubbleLayouts_.size();
}

int UnassignedLayoutElements::numElements() const {
	return numQuestionGroups() + numQuestions() + numBubbles();
}

QuestionGroupLayout * UnassignedLayoutElements::getQuestionGroupLayout(int index) {
	//Get the relative index of the question group
	int relativeIndex = questionGroupIndex(index);

	//Retrieve the requested question group
	struct QuestionGroupLayout* questionGroupLayout = nullptr;
	if(relativeIndex >= 0) {
		questionGroupLayout = &questionGroupLayouts_[relativeIndex];
	} else {
		tlOss << "Invalid question group index " << index << " was requested.";
		tlog.critical(__FILE__, __LINE__, tlOss);
	}
	return questionGroupLayout;
}

QuestionLayout * UnassignedLayoutElements::getQuestionLayout(int index) {
	//Get the relative index of the question
	int relativeIndex = questionIndex(index);

	//Retrieve the requested question
	struct QuestionLayout* questionLayout = nullptr;
	if(relativeIndex >= 0) {
		questionLayout = &questionLayouts_[relativeIndex];
	} else {
		tlOss << "Invalid question index " << index << " was requested.";
		tlog.critical(__FILE__, __LINE__, tlOss);
	}
	return questionLayout;
}

BubbleLayout * UnassignedLayoutElements::getBubbleLayout(int index) {
	//Get the relative index of the bubble
	int relativeIndex = bubbleIndex(index);

	//Retrieve the requested bubble
	struct BubbleLayout* bubbleLayout = nullptr;
	if(relativeIndex >= 0) {
		bubbleLayout = &bubbleLayouts_[relativeIndex];
	} else {
		tlOss << "Invalid bubble index " << index << " was requested.";
		tlog.critical(__FILE__, __LINE__, tlOss);
	}
	return bubbleLayout;
}

QList<QTreeWidgetItem*> UnassignedLayoutElements::treeWidgetItem() {
	QList<QTreeWidgetItem*> treeItems;

	//Generate tree items for unassigned question groups
	for(const auto& questionGroupLayout : questionGroupLayouts_) {
		treeItems.push_back(LayoutTreeBuilder::generateTreeItem(questionGroupLayout));
	}

	//Generate tree ietms for unassigned questoins
	for(const auto& questionLayout : questionLayouts_) {
		treeItems.push_back(LayoutTreeBuilder::generateTreeItem(questionLayout));
	}

	//Generate tree items for unassigned bubbles
	for(const auto& bubbleLayout : bubbleLayouts_) {
		treeItems.push_back(LayoutTreeBuilder::generateTreeItem(bubbleLayout));
	}

	return treeItems;
}

int UnassignedLayoutElements::questionGroupIndex(int absoluteIndex) {
	//Question groups are first, so there is no need to subtract anything to get the relative index
	return absoluteIndex < numQuestionGroups() ? absoluteIndex : -1;
}

int UnassignedLayoutElements::questionIndex(int absoluteIndex) {
	//Questions come after question groups, so the first question indices come after the last question group indices
	int relativeIndex = absoluteIndex - numQuestionGroups();
	return relativeIndex < numQuestions() ? relativeIndex : -1;
}

int UnassignedLayoutElements::bubbleIndex(int absoluteIndex) {
	//Bubbles come after question groups and questions, so the first bubble index comes after the last question index (which comes after the last question group index)
	int relativeIndex = absoluteIndex - numQuestionGroups() - numQuestions();
	return relativeIndex < numBubbles() ? relativeIndex : -1;
}
