
#include "SheetLayoutTreeBuilder.hxx"

#include <sstream>

QTreeWidgetItem* LayoutTreeBuilder::generateTreeItem(const SideLayout& side, QTreeWidgetItem * parent) {

	//Piece together the appropriate name for the side tree item. sideNumber + 1 is used because internally the sides start at zero but the tree in the GUI should start at 1
	std::ostringstream sideName;
	sideName << "Side " << side.sideNumber_ + 1 << " Layout";

	//Create the tree widget item to represent the side itself
	QTreeWidgetItem* sideItem = new QTreeWidgetItem(parent, QStringList(QString::fromStdString(sideName.str())), (int)TreeItemType::SIDE_LAYOUT);

	//Generate tree items for each question group on side and add them as children of the question group item
	for(const auto& questionGroup : side.questionGroups_) {
		generateTreeItem(questionGroup, sideItem);
	}

	return sideItem;
}

QTreeWidgetItem* LayoutTreeBuilder::generateTreeItem(const QuestionGroupLayout& questionGroup, QTreeWidgetItem* parent) {
	//Create the tree widget item to represent the question group itself
	QTreeWidgetItem* questionGroupItem = new QTreeWidgetItem(parent, QStringList(QString::fromStdString(questionGroup.name_)), (int)TreeItemType::QUESTION_GROUP_LAYOUT);

	//Generate tree items for each question in the question group and add them as children of the question group item
	for(const auto& question : questionGroup.questions_) {
		generateTreeItem(question, questionGroupItem);
	}

	return questionGroupItem;
}

QTreeWidgetItem* LayoutTreeBuilder::generateTreeItem(const QuestionLayout& question, QTreeWidgetItem* parent) {

	//Piece together the display name for the question group. Using questionNumber + 1 so that the numbering of questions displayed in the GUI starts at 1 not 0
	std::ostringstream questionName;
	questionName << "Question " << question.questionNumber_ + 1;

	//Create the tree widget to represent the question itself
	QTreeWidgetItem* questionItem = new QTreeWidgetItem(parent, QStringList(QString::fromStdString(questionName.str())), (int)TreeItemType::QUESTION_LAYOUT);

	//Generate tree items for each of the bubbles in the question and add them as children of the question item
	for(const auto& bubble : question.bubbles_) {
		generateTreeItem(bubble, questionItem);
	}

	return questionItem;
}

QTreeWidgetItem* LayoutTreeBuilder::generateTreeItem(const BubbleLayout& bubble, QTreeWidgetItem* parent) {
	//Create the tree widget item to represent the bubble.
	QTreeWidgetItem* bubbleItem = new QTreeWidgetItem(parent, QStringList("\"" + QString::fromStdString(bubble.answer_) + "\" bubble"), (int)TreeItemType::BUBBLE_LAYOUT);
	//Note: bubbles are always leaf items, there is no need to generate items for their children
	return bubbleItem;
}
