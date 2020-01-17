// 
// University of Massachusetts Amherst Physics Lecture Prep
//
// Project: ScantronReader
// File: SheetLayoutTreeBuilder.hxx
// Author: Casey Massar (caseymassar@gmail.com)
// Description: Utility class used by the sheet layout editor dialog to generate the sheet layout tree display.
//

#pragma once

#include <QTreeWidgetItem>

#include "SheetLayout.hxx"


// The layout tree display keeps track of the type of each item (using values from this enum) in order to help trace it back to a specific entity in the SheetLayout object.
enum class TreeItemType : int {
	UNKNOWN = QTreeWidgetItem::UserType,
	SIDE_LAYOUT,
	QUESTION_GROUP_LAYOUT,
	QUESTION_LAYOUT,
	BUBBLE_LAYOUT,
	UNASSIGNED_ITEM_LIST
};

class LayoutTreeBuilder {
public:
	static QTreeWidgetItem* generateTreeItem(const SideLayout& side, QTreeWidgetItem* parent = nullptr);
	static QTreeWidgetItem* generateTreeItem(const QuestionGroupLayout& questionGroup, QTreeWidgetItem* parent = nullptr);
	static QTreeWidgetItem* generateTreeItem(const QuestionLayout& question, QTreeWidgetItem* parent = nullptr);
	static QTreeWidgetItem* generateTreeItem(const BubbleLayout& bubble, QTreeWidgetItem* parent = nullptr);
};