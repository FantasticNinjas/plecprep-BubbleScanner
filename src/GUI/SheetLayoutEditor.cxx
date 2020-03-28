#include <QDir>
#include <QMessageBox>
#include <QScrollBar>
#include <QDebug>

#include <sstream>
#include <fstream>

#include "SheetLayoutEditor.hxx"
#include "ui_SheetLayoutEditor.h"

#include "FilenameOracle.hxx"
#include "QtLogging.hxx"

namespace {
	std::ostringstream tlOss;
	QtLogging qlog;
}

SheetLayoutEditor::SheetLayoutEditor(QWidget *parent) : QDialog(parent) {
	//Default UI setup
	ui = new Ui::SheetLayoutEditor();
	ui->setupUi(this);
	//This is a little bit of a hack. QT Designer doesn't have a good way to set the widget in a scroll area to a label, so instead the label is placed somewhere random in the .ui file and then moved to the scroll area here.
	ui->imageScrollArea->setWidget(ui->imageLabel);
	//Make the image viewer background dark.
	ui->imageScrollArea->setBackgroundRole(QPalette::Dark);
	//Populate lists of available image processing algorithms (under Sheet Layout Tools)

	//Set property editors (such as bubble x position) to only accept numbers
	ui->bubbleRadiusEdit->setValidator(new QDoubleValidator(0.0, 10.0, 20, this));
	ui->bubbleXEdit->setValidator(new QDoubleValidator(0.0, 10.0, 20, this));
	ui->bubbleYEdit->setValidator(new QDoubleValidator(0.0, 10.0, 20, this));

	ui->boxSelectLeftBound->setValidator(new QDoubleValidator(0.0, 10.0, 20, this));
	ui->boxSelectRightBound->setValidator(new QDoubleValidator(0.0, 10.0, 20, this));
	ui->boxSelectTopBound->setValidator(new QDoubleValidator(0.0, 10.0, 20, this));
	ui->boxSelectBottomBound->setValidator(new QDoubleValidator(0.0, 10.0, 20, this));

	ui->questionNumberEdit->setValidator(new QIntValidator(-1, 1000));

	reloadAlgorithmList();
	//Populate the sheet layout selector
	if(reloadLayoutList() >= 0) {
		openSelectedLayout();
	}
}

SheetLayoutEditor::~SheetLayoutEditor() {
	delete ui;
}

void SheetLayoutEditor::on_layoutChooser_activated(const QString &text) {
	openSelectedLayout();
}

void SheetLayoutEditor::on_layoutTree_itemSelectionChanged() {
	//Reset focused items;
	SideLayout* focusedSideLayout = nullptr;
	GroupLayout* focusedGroupLayout = nullptr;
	QuestionLayout* focusedQuestionLayout = nullptr;
	BubbleLayout* focusedBubbleLayout = nullptr;

	//Search through the currently selected items and select one of each type to be the focus
	for(const auto treeItem : ui->layoutTree->selectedItems()) {
		switch(treeItem->type()) {
		case (int)TreeItemType::BUBBLE_LAYOUT:
			focusedBubbleLayout = dynamic_cast<BubbleLayout*>(findLayoutElement(treeItem));
			break;
		case (int)TreeItemType::QUESTION_LAYOUT:
			focusedQuestionLayout = dynamic_cast<QuestionLayout*>(findLayoutElement(treeItem));
			break;
		case (int)TreeItemType::QUESTION_GROUP_LAYOUT:
			focusedGroupLayout = dynamic_cast<GroupLayout*>(findLayoutElement(treeItem));
			break;
		case (int)TreeItemType::SIDE_LAYOUT:
			focusedSideLayout = dynamic_cast<SideLayout*>(findLayoutElement(treeItem));
			break;
		}
	}

	//Open editor image
	if(focusedSideLayout != nullptr) {
		openEditorImage(FilenameOracle::getLayoutDirectoryFilename() + FilenameOracle::getImageDirectory() + focusedSideLayout->getReferenceImageFilename());
		//Update the editor image to reflect the change in selection
	}
	reloadEditorImage();
	//Update toolbox fields
	updateBubbleEditor();
	updateQuestionEditor();
	updateGroupEditor();
}

void SheetLayoutEditor::on_zoomOutButton_clicked() {
	zoomEditorImage(0.8);
}

void SheetLayoutEditor::on_zoomInButton_clicked() {
	zoomEditorImage(1.25);
}

void SheetLayoutEditor::on_alignBackgroundButton_clicked() {
	int status = 0;

	//Check that the sheet image exists
	if(editorImage_.empty()) {
		tlOss << "Not aligning background because editor image is not loaded.";
		qlog.debug(__FILE__, __LINE__, this, tlOss);
		status = 1;
	}

	//Get the name of the current image alignment algorithm from the algorithm chooser
	std::string algorithmName = ui->alignmentAlgoChooser->currentText().toStdString();
	if(status == 0 && algorithmName.empty()) {
		tlOss << "Not aligning background image because no alignment algorithm is selected.";
		qlog.debug(__FILE__, __LINE__, this, tlOss);
		status = 1;
	}

	//Load the requested algorithm configuration
	DetectionParams algorithmParams;
	if(status == 0) {
		if(algorithmParams.load(FilenameOracle::getAlignmentAlgorithmsFilename(), algorithmName) < 0) {
			status = -1;
			tlOss << "Failed to load sheet alignment algorithm \"" << algorithmName << "\"";
			qlog.critical(__FILE__, __LINE__, this, tlOss);
		}
	}

	//Apply the initialization step of the algorithm 
	if(status == 0) {
		if(editorImage_.setupAlgorithm(algorithmParams) < 0) {
			status = -1;
			tlOss << "Failed to initialize alignment algorithm \"" << algorithmName << "\"";
			qlog.critical(__FILE__, __LINE__, this, tlOss);
		}
	}

	//Apply the main step of the algorithm
	if(status == 0) {
		if(editorImage_.alignScan(algorithmParams) < 0) {
			status = -1;
			tlOss << "Failed to align image.";
			qlog.critical(__FILE__, __LINE__, this, tlOss);
		}
	}

	//Update the UI to reflect the changed sheet scan.
	reloadEditorImage();
}

void SheetLayoutEditor::on_recognizeCirclesButton_clicked() {
	int status = 0;

	//Check that the sheet image exists
	if(editorImage_.empty()) {
		tlOss << "Not running circle recognition algorithm because editor image is not loaded.";
		qlog.debug(__FILE__, __LINE__, this, tlOss);
		status = 1;
	}

	//Get the circle recognition algorithm to use from the algorithm chooser
	std::string algorithmName = ui->circleAlgoPicker->currentText().toStdString();
	if(status == 0 && algorithmName.empty()) {
		tlOss << "Not running circle recognition algorithm because no algorithm is selected.";
		qlog.debug(__FILE__, __LINE__, this, tlOss);
		status = 1;
	}

	//Load the requested algorithm configuration
	DetectionParams algorithmParams;
	if(status == 0) {
		if(algorithmParams.load(FilenameOracle::getCircleAlgorithmsFilename(), algorithmName) < 0) {
			status = -1;
			tlOss << "Failed to load circle recognition algorithm \"" << algorithmName << "\"";
			qlog.critical(__FILE__, __LINE__, this, tlOss);
		}
	}

	//Apply the initialization step of the algorithm 
	if(status == 0) {
		if(editorImage_.setupAlgorithm(algorithmParams) < 0) {
			status = -1;
			tlOss << "Failed to initialize circle recognition algorithm \"" << algorithmName << "\"";
			qlog.critical(__FILE__, __LINE__, this, tlOss);
		}
	}

	std::vector<cv::Vec3f> circles;
	if(status == 0) {
		if(editorImage_.findCircles(circles, algorithmParams) < 0) {
			status = -1;
			tlOss << "Circle recognition algorithm reported an error.";
			qlog.critical(__FILE__, __LINE__, this, tlOss);
		}
	}

	if(status == 0) {
		for(const auto& circle : circles) {
			BubbleLayout bubble;
			bubble.setLocation(circle);
			unownedLayoutElements.add(bubble);
		}
		buildLayoutTree();
		reloadEditorImage();
	}
	
}

void SheetLayoutEditor::on_bubbleTextEdit_editingFinished() {
	applyBubbleEditor();
}

void SheetLayoutEditor::on_bubbleXEdit_editingFinished() {
	applyBubbleEditor();
}

void SheetLayoutEditor::on_bubbleYEdit_editingFinished() {
	applyBubbleEditor();
}

void SheetLayoutEditor::on_bubbleRadiusEdit_editingFinished() {
	applyBubbleEditor();
}

void SheetLayoutEditor::on_addBubble_clicked() {
	unownedLayoutElements.add(BubbleLayout());
	buildLayoutTree();
	reloadEditorImage();
}

void SheetLayoutEditor::on_deleteBubbles_clicked() {

	//Search through the currently selected items for bubbles to delete
	QList<QTreeWidgetItem*> selectedBubbles;
	for(const auto& treeItem : ui->layoutTree->selectedItems()) {
		if(treeItem->type() == static_cast<int>(TreeItemType::BUBBLE_LAYOUT)) {
			selectedBubbles.append(treeItem);
		}
	}

	removeLayoutElements(selectedBubbles);
	buildLayoutTree();
}

void SheetLayoutEditor::on_questionFromBubbles_clicked() {
	//Make a new question layout
	QuestionLayout newQuestion;

	//Search through the currently selected items for bubbles to be added to the new question (and also add them to a QList of said bubbles so they can be removed from wherever they were before.)
	QList<QTreeWidgetItem*> selectedBubbles;
	for(const auto& treeItem : ui->layoutTree->selectedItems()) {
		if(treeItem->type() == static_cast<int>(TreeItemType::BUBBLE_LAYOUT)) {
			BubbleLayout* bubble_ptr = dynamic_cast<BubbleLayout*>(findLayoutElement(treeItem));
			if(bubble_ptr == nullptr) {
				tlOss << "Attempted to add non-existant bubble to new question";
				qlog.critical(__FILE__, __LINE__, this, tlOss);
			} else {
				newQuestion.addBubble(bubble_ptr);
				selectedBubbles.append(treeItem);
			}
		}
	}

	//Remove bubbles that were added to the new question
	removeLayoutElements(selectedBubbles);

	//Add new question layout to the list of unowned layout elements
	unownedLayoutElements.add(newQuestion);

	//rebuild layout tree
	buildLayoutTree();
}

void SheetLayoutEditor::on_groupNameInput_editingFinished() {
	applyGroupEditor();
}

void SheetLayoutEditor::on_groupToSide_clicked() {
	//Find the selected side layout
	SideLayout* focusedSide_ptr = nullptr;
	for(auto treeItem_ptr : ui->layoutTree->selectedItems()) {
		if(treeItem_ptr->type() == static_cast<int>(TreeItemType::SIDE_LAYOUT)) {
			SideLayout* side_ptr = dynamic_cast<SideLayout*>(findLayoutElement(treeItem_ptr));
			if(side_ptr != nullptr) {
				focusedSide_ptr = side_ptr;
			}
		}
	}

	//Find all of the selected question groups and add them to the selected side
	if(focusedSide_ptr == nullptr) {
		tlOss << "Attempted to add group layouts to nonexistant side";
		qlog.critical(__FILE__, __LINE__, this, tlOss);
	} else {
		QList<QTreeWidgetItem*> selectedGroups;
		for(const auto& treeItem : ui->layoutTree->selectedItems()) {
			if(treeItem->type() == static_cast<int>(TreeItemType::QUESTION_GROUP_LAYOUT)) {
				GroupLayout* group_ptr = dynamic_cast<GroupLayout*>(findLayoutElement(treeItem));
				if(group_ptr == nullptr) {
					tlOss << "Attempted to add non-existant group to new side";
					qlog.critical(__FILE__, __LINE__, this, tlOss);
				} else {
					focusedSide_ptr->addGroup(group_ptr);
					selectedGroups.append(treeItem);
				}
			}
		}

		//Remove groups that were added to the side from wherever they used to be
		removeLayoutElements(selectedGroups);
	}
	buildLayoutTree();
}

void SheetLayoutEditor::on_questionNumberEdit_editingFinished() {
	applyQuestionEditor();
}

void SheetLayoutEditor::on_groupFromQuestions_clicked() {
	//Make a new group layout
	GroupLayout newGroup;

	//Search through the currently selected items for questions to be added to the new group (and also add them to a QList of said questions so they can be removed from wherever they were before.)
	QList<QTreeWidgetItem*> selectedQuestions;
	for(const auto& treeItem : ui->layoutTree->selectedItems()) {
		if(treeItem->type() == static_cast<int>(TreeItemType::QUESTION_LAYOUT)) {
			QuestionLayout* question_ptr = dynamic_cast<QuestionLayout*>(findLayoutElement(treeItem));
			if(question_ptr == nullptr) {
				tlOss << "Attempted to add non-existant question to new group";
				qlog.critical(__FILE__, __LINE__, this, tlOss);
			} else {
				newGroup.addQuestion(question_ptr);
				selectedQuestions.append(treeItem);
			}
		}
	}

	//Remove questions that were added to the new group
	removeLayoutElements(selectedQuestions);

	//Add new group to the list of unowned layout elements
	unownedLayoutElements.add(newGroup);

	//Rebuild layout tree
	buildLayoutTree();
}

void SheetLayoutEditor::on_boxSelectActivate_clicked() {
	//Create a cv::Rect2f matching the selection bounds specified in the GUI
	float leftBound = std::stof(ui->boxSelectLeftBound->text().toStdString());
	float rightBound = std::stof(ui->boxSelectRightBound->text().toStdString());
	float topBound = std::stof(ui->boxSelectTopBound->text().toStdString());
	float bottomBound = std::stof(ui->boxSelectBottomBound->text().toStdString());
	cv::Rect2f selectionBox(cv::Point2f(leftBound, topBound), cv::Point2f(rightBound, bottomBound));

	//Select all bubbles in the box
	boxSelection(selectionBox);
}

void SheetLayoutEditor::on_saveButton_clicked() {
	int status = 0;

	//Find the name of the currently open sheet layout
	std::string layoutTitle = currentLayout_.getTitle();

	//Check that the layout to load is in the list of layouts.
	if(layouts_.find(layoutTitle) == layouts_.end()) {
		status = -1;
		tlOss << "Unrecognized sheet layout title \"" << layoutTitle << "\".";
		qlog.critical(__FILE__, __LINE__, this, tlOss);
	}

	//Write the current state of the sheet layout to the appropriate file
	if(status >= 0) {
		QFileInfo sheetLayoutFile(QString::fromStdString(layouts_[layoutTitle]));
		std::ofstream sheetLayoutStream(sheetLayoutFile.absoluteFilePath().toStdString());
		currentLayout_.writeXml(sheetLayoutStream);
		sheetLayoutStream.close();
	}
}

int SheetLayoutEditor::reloadLayoutList() {
	//Get a list of all of the readable xml files in the sheet layout directory
	int status = 0;

	//Remove any existing items in the layout picker
	ui->layoutChooser->clear();
	layouts_.clear();

	QDir sheetLayoutsDirectory(QString::fromStdString(FilenameOracle::getLayoutDirectoryFilename()), "*.xml");
	QFileInfoList layoutFiles = sheetLayoutsDirectory.entryInfoList();

	tlOss << "Found " << layoutFiles.size() << " sheet layout files.";
	qlog.debug(__FILE__, __LINE__, this, tlOss);

	//Get the title of each sheet layout in the list and add it to the drop down menu as well as the list of sheet layouts
	for(const auto& fileInfo : layoutFiles) {
		//Open the scan sheet layout file for reading
		std::ifstream sheetLayoutStream(fileInfo.absoluteFilePath().toStdString());
		
		//Read the sheet layout file into a scan sheet layout
		ScanSheetLayout currentLayout;
		if(currentLayout.readXml(sheetLayoutStream) < 0) {
			tlOss << "Failed to read sheet layout file \"" << fileInfo.fileName().toStdString() << "\"";
			qlog.warning(__FILE__, __LINE__, this, tlOss);
		} else {
			//Add this sheet layout to the layout picker box
			std::string layoutTitle = currentLayout.getTitle();
			ui->layoutChooser->addItem(QString::fromStdString(layoutTitle));
			layouts_[layoutTitle] = fileInfo.absoluteFilePath().toStdString();
		}
		sheetLayoutStream.close();
	}

	//Add new sheet layout option to the list.
	ui->layoutChooser->addItem("<Create New Sheet Layout>");

	return status;
}

void SheetLayoutEditor::openSelectedLayout() {
	int status = 0;

	if(ui->layoutChooser->currentIndex() < ui->layoutChooser->count() - 1) {
		if(openLayout(ui->layoutChooser->currentText().toStdString()) < 0) {
			status = -1;
		}
	}
}

int SheetLayoutEditor::openLayout(const std::string& layoutTitle) {
	int status = 0;

	//Check that the layout to load is in the list of layouts.
	if(layouts_.find(layoutTitle) == layouts_.end()) {
		status = -1;
		tlOss << "Unrecognized sheet layout title \"" << layoutTitle << "\".";
		qlog.critical(__FILE__, __LINE__, this, tlOss);
	}

	//Load the sheet layout
	if(status >= 0) {
		QFileInfo sheetLayoutFile(QString::fromStdString(layouts_[layoutTitle]));
		std::ifstream sheetLayoutStream(sheetLayoutFile.absoluteFilePath().toStdString());
		if(currentLayout_.readXml(sheetLayoutStream) < 0) {
			status = -1;
			tlOss << "Failed to load sheet layout \"" << layoutTitle << "\" from \"" << layouts_[layoutTitle] << "\"";
			qlog.critical(__FILE__, __LINE__, this, tlOss);
		}
		sheetLayoutStream.close();
	}

	//Rebuild the layout tree display based on the new scan sheet layout
	if(status >= 0) {
		status = buildLayoutTree();
	}

	//Update the editor background image based on the new scan sheet layout
	if(status >= 0) {
		status = reloadEditorImage();
	}

	return status;
}

int SheetLayoutEditor::buildLayoutTree() {
	int status = 0;

	//Clear out any items already in the tree
	ui->layoutTree->clear();

	//Create tree items to represent the sheet layout elements on each side of the scan sheet
	for(int i = 0; i < currentLayout_.numSides(); i++) {
		ui->layoutTree->addTopLevelItem(buildTreeWidget(currentLayout_.sideLayout(i)));
	}
	

	//Create tree items to represent unassigned layout elements
	QTreeWidgetItem* unownedElementsItem = new QTreeWidgetItem((QTreeWidgetItem*)nullptr, QStringList("Unassigned Elements"), (int)TreeItemType::UNASSIGNED_ITEM_LIST);
	
	for(int i = 0; i < unownedLayoutElements.size(); i++) {
		buildTreeWidget(unownedLayoutElements.elementAt(i), unownedElementsItem);
	}

	//Add unowned layout elements to tree
	ui->layoutTree->addTopLevelItem(unownedElementsItem);

	return status;
}

QTreeWidgetItem* SheetLayoutEditor::buildTreeWidget(SheetLayoutElement* layoutElement, QTreeWidgetItem* parent) {

	//Find the type of this sheet element
	TreeItemType itemType = TreeItemType::UNKNOWN;
	if(dynamic_cast<BubbleLayout*>(layoutElement) != nullptr) {
		itemType = TreeItemType::BUBBLE_LAYOUT;
	} else if(dynamic_cast<QuestionLayout*>(layoutElement) != nullptr) {
		itemType = TreeItemType::QUESTION_LAYOUT;
	} else if(dynamic_cast<GroupLayout*>(layoutElement) != nullptr) {
		itemType = TreeItemType::QUESTION_GROUP_LAYOUT;
	} else if(dynamic_cast<SideLayout*>(layoutElement) != nullptr) {
		itemType = TreeItemType::SIDE_LAYOUT;
	}

	//Create the tree item for this layout element
	QTreeWidgetItem* current = new QTreeWidgetItem(parent, QStringList(QString::fromStdString(layoutElement->toString())), static_cast<int>(itemType));

	//Generate tree items for the children of this layout element
	for(int i = 0; i < layoutElement->numChildren(); i++) {
		buildTreeWidget(layoutElement->childAt(i), current);
	}

	return current;
}

int SheetLayoutEditor::openEditorImage(const std::string& filename) {
	int status = 0;

	//Load the image specified by filename
	if(editorImage_.load(filename) < 0) {
		status = -1;
		tlOss << "Unable to open image file \"" << filename << "\", it may be missing or corrupt.";
		qlog.critical(__FILE__, __LINE__, this, tlOss);
	}

	//Display the image in the editor
	if(status >= 0 && reloadEditorImage() < 0) {
		status = -1;
		tlOss << "Image loaded from \"" << filename << "\" (associated with this sheet layout) appears to be empty.";
		qlog.warning(__FILE__, __LINE__, this, tlOss);
	}


	return status;
}

void SheetLayoutEditor::annotateEditorImage() {
	int status = 0;
	//If the editor image is empty, there is nothing to annotate, so just return
	if(editorImage_.empty()) {
		tlOss << "Not annotating sheet layout editor image, editorImage is not loaded.";
		qlog.debug(__FILE__, __LINE__, this, tlOss);
		status = 1;
	}

	if(status <= 0) {
		//Reset all annotations on the layout background image
		editorImage_.resetAnnotations();

		//Draw each visible item in the layout tree
		for(QTreeWidgetItem* item = ui->layoutTree->topLevelItem(0); item != nullptr; item = ui->layoutTree->itemBelow(item)) {
			//Set the color to draw the layout element based on whether or not its item is selected.
			cv::Scalar color(255, 0, 0);
			if(item->isSelected()) {
				color = cv::Scalar(0, 0, 255);
			}
			 
			//Draw the layout element

			if(item->type() == (int)TreeItemType::BUBBLE_LAYOUT) {
				BubbleLayout* bubble = dynamic_cast<BubbleLayout*>(findLayoutElement(item));
				if(bubble != nullptr) {
					editorImage_.annotateCircle(bubble->getLocation(), color, 2);
				}
			}
		}
	}
}

void SheetLayoutEditor::boxSelection(cv::Rect2f & selectionBox) {
	//Note: This function works by iterating over the unassigned layout elements in the layout tree display, not in the UnassignedLayoutElements instance itself. This is because there is currently a method for
	//going from an item in the layout tree display to its corrisponding layout element, but not the other way around. This is important because which elements are selected is stored in the layout tree display,
	//while the positions of elements (and therefore whether they should be selected) are stored in the layout elements themselves, so this method needs both.

	int status = 0;

	//Find the unassigned elemts item in the layout tree
	QTreeWidgetItem* unassignedElementsItem = nullptr;
	if(status >= 0) {
		for(int i = 0; i < ui->layoutTree->topLevelItemCount(); i++) {
			QTreeWidgetItem* currentItem = ui->layoutTree->topLevelItem(i);
			if(currentItem->type() == (int)TreeItemType::UNASSIGNED_ITEM_LIST) {
				unassignedElementsItem = currentItem;
				break;
			}
		}

		if(unassignedElementsItem == nullptr) {
			status = -1;
			tlOss << "Failed to find unassigned layout elements tree item";
			qlog.critical(__FILE__, __LINE__, this, tlOss);
		}

	}

	if(status >= 0) {
		//Iterate over all unowned layout elements
		for(int i = 0; i < unassignedElementsItem->childCount(); i++) {
			QTreeWidgetItem* item = unassignedElementsItem->child(i);
			SheetLayoutElement* element = findLayoutElement(item);
			if(element == nullptr) {
				tlOss << "Encountered invalid sheet layout element while performing box selection.";
				qlog.warning(__FILE__, __LINE__, this, tlOss);
			} else {
				if((selectionBox & element->boundingBox()).area() > 0) {
					item->setSelected(true);
				}
			}
		}
	}

}

int SheetLayoutEditor::reloadEditorImage() {
	int status = 0;

	annotateEditorImage();

	//Get the pixmap to display from the SheetScan.
	QPixmap editorPixmap = editorImage_.getAnnotatedPixmap();
	if(editorPixmap.isNull()) {
		status = -1;
	}

	if(status == 0) {
		//Display the new pixmap in the image label
		ui->imageLabel->setPixmap(editorPixmap);
		//Resize the image label (in case the pixmap is a different size).
		zoomEditorImage();
	}

	return status;
}

int SheetLayoutEditor::resetEditorImageScale() {
	return zoomEditorImage(1.0f / editorImageScale_);
}

int SheetLayoutEditor::zoomEditorImage(float amount) {
	int status = 0;

	if(amount <= 0) {
		tlOss << "Invalid scale amount: " << amount;
		qlog.warning(__FILE__, __LINE__, this, tlOss);
	}

	if(status >= 0) {
		//Adjust the size of the image label
		editorImageScale_ *= amount;
		
		ui->imageLabel->setFixedSize(editorImageScale_ * ui->imageLabel->pixmap()->size());

		//Reposition scroll bars to keep the center in the center
		ui->imageScrollArea->horizontalScrollBar()->setValue(int(amount * ui->imageScrollArea->horizontalScrollBar()->value() + ((amount - 1) * ui->imageScrollArea->horizontalScrollBar()->pageStep() / 2)));
		ui->imageScrollArea->verticalScrollBar()->setValue(int(amount * ui->imageScrollArea->verticalScrollBar()->value() + ((amount - 1) * ui->imageScrollArea->verticalScrollBar()->pageStep() / 2)));
	}

	return status;
}

int SheetLayoutEditor::reloadAlgorithmList() {
	int status = 0;

	ui->alignmentAlgoChooser->clear();
	ui->circleAlgoPicker->clear();

	//Get the list of alignment algorithms
	std::vector<std::string> alignmentAlgorithms;
	if(DetectionParams::getFilterList(FilenameOracle::getAlignmentAlgorithmsFilename(), alignmentAlgorithms) >= 0) {
		tlOss << "Found " << alignmentAlgorithms.size() << " image alignment algorithms";
		qlog.debug(__FILE__, __LINE__, this, tlOss);
	} else {
		status = -1;
		tlOss << "Failed to load background image alignment algorithms.";
		qlog.warning(__FILE__, __LINE__, this, tlOss);
	}

	//Add alignment algorithms to the combo box
	if(status >= 0) {
		for(const auto& algorithm : alignmentAlgorithms) {
			ui->alignmentAlgoChooser->addItem(QString::fromStdString(algorithm));
		}
	}

	//Get the list of circle detection algorithms
	std::vector<std::string> circleAlgorithms;
	if(status >= 0) {
		if(DetectionParams::getFilterList(FilenameOracle::getCircleAlgorithmsFilename(), circleAlgorithms) >= 0) {
			tlOss << "Found " << circleAlgorithms.size() << " circle detection algorithms";
			qlog.debug(__FILE__, __LINE__, this, tlOss);
		} else {
			status = -1;
			tlOss << "Failed to load circle recognition alignment algorithms.";
			qlog.warning(__FILE__, __LINE__, this, tlOss);
		}
	}

	//Add circle algorithms to combo box
	if(status >= 0) {
		for(const auto& algorithm : circleAlgorithms) {
			ui->circleAlgoPicker->addItem(QString::fromStdString(algorithm));
		}
	}

	return status;
}

void SheetLayoutEditor::updateBubbleEditor() {

	resetbubbleEditor();

	//Find the number of selected bubbles as well as the bounding box of all selected bubbles
	size_t numBubbles = 0;
	BubbleLayout* focusedBubble_ptr = nullptr;
	cv::Rect2f boundingBox;
	for(auto treeItem_ptr : ui->layoutTree->selectedItems()) {
		if(treeItem_ptr->type() == static_cast<int>(TreeItemType::BUBBLE_LAYOUT)) {
			BubbleLayout* bubble_ptr = dynamic_cast<BubbleLayout*>(findLayoutElement(treeItem_ptr));
			if(bubble_ptr != nullptr) {
				focusedBubble_ptr = bubble_ptr;
				boundingBox = boundingBox | bubble_ptr->boundingBox();
				numBubbles++;
			}
		}
	}

	//Update individual bubble fields with bubble-specific information if there is only one bubble selected

	if(numBubbles == 1) {
		//Set bubble text
		ui->bubbleTextEdit->setText(QString::fromStdString(focusedBubble_ptr->getAnswer()));
		//Set bubble x coordinate
		std::ostringstream xCoordStream;
		xCoordStream << focusedBubble_ptr->getLocation()[0];
		ui->bubbleXEdit->setText(QString::fromStdString(xCoordStream.str()));
		//Set bubble y coordinate
		std::ostringstream yCoordStream;
		yCoordStream << focusedBubble_ptr->getLocation()[1];
		ui->bubbleYEdit->setText(QString::fromStdString(yCoordStream.str()));
		//Set bubble radius
		std::ostringstream radiusStream;
		radiusStream << focusedBubble_ptr->getLocation()[2];
		ui->bubbleRadiusEdit->setText(QString::fromStdString(radiusStream.str()));
	}

	if(numBubbles >= 1) {
		//Set box selection bounds to boundingbox

		std::ostringstream leftBoundStream;
		leftBoundStream << boundingBox.x;
		ui->boxSelectLeftBound->setText(QString::fromStdString(leftBoundStream.str()));

		std::ostringstream rightBoundStream;
		rightBoundStream << boundingBox.x + boundingBox.width;
		ui->boxSelectRightBound->setText(QString::fromStdString(rightBoundStream.str()));

		std::ostringstream topBoundStream;
		topBoundStream << boundingBox.y;
		ui->boxSelectTopBound->setText(QString::fromStdString(topBoundStream.str()));

		std::ostringstream bottomBoundStream;
		bottomBoundStream << boundingBox.y + boundingBox.height;
		ui->boxSelectBottomBound->setText(QString::fromStdString(bottomBoundStream.str()));
	}
}

void SheetLayoutEditor::applyBubbleEditor() {

	//Update changed fields on all selected bubbles

	for(auto treeItem_ptr : ui->layoutTree->selectedItems()) {
		if(treeItem_ptr->type() == static_cast<int>(TreeItemType::BUBBLE_LAYOUT)) {
			BubbleLayout* bubble_ptr = dynamic_cast<BubbleLayout*>(findLayoutElement(treeItem_ptr));
			if(bubble_ptr != nullptr) {
				std::string answer = ui->bubbleTextEdit->text().toStdString();
				if(!answer.empty()) {
					bubble_ptr->setAnswer(answer);
				}

				if(!ui->bubbleXEdit->text().isEmpty()) {
					bubble_ptr->setCenterX(ui->bubbleXEdit->text().toFloat());
				}

				if(!ui->bubbleYEdit->text().isEmpty()) {
					bubble_ptr->setCenterY(ui->bubbleYEdit->text().toFloat());
				}

				if(!ui->bubbleRadiusEdit->text().isEmpty()) {
					bubble_ptr->setRadius(ui->bubbleRadiusEdit->text().toFloat());
				}
			}
		}
	}

	buildLayoutTree();
	reloadEditorImage();
}

void SheetLayoutEditor::resetbubbleEditor() {
	ui->bubbleTextEdit->setText("");
	ui->bubbleXEdit->setText("");
	ui->bubbleYEdit->setText("");
	ui->bubbleRadiusEdit->setText("");
	ui->boxSelectBottomBound->setText("");
	ui->boxSelectTopBound->setText("");
	ui->boxSelectLeftBound->setText("");
	ui->boxSelectRightBound->setText("");
}

void SheetLayoutEditor::updateQuestionEditor() {
	resetQuestionEditor();

	//Find the number of selected questions
	size_t numQuestions = 0;
	QuestionLayout* focusedQuestion_ptr = nullptr;
	for(auto treeItem_ptr : ui->layoutTree->selectedItems()) {
		if(treeItem_ptr->type() == static_cast<int>(TreeItemType::QUESTION_LAYOUT)) {
			QuestionLayout* question_ptr = dynamic_cast<QuestionLayout*>(findLayoutElement(treeItem_ptr));
			if(question_ptr != nullptr) {
				focusedQuestion_ptr = question_ptr;
				numQuestions++;
			}
		}
	}

	//If there is only one selected question, update text boxes to display question-specific information
	if(numQuestions == 1) {
		//Only display the question number if it has been set
		if(focusedQuestion_ptr->getQuestionNumber() >= 0) {
			ui->questionNumberEdit->setText(QString::number(focusedQuestion_ptr->getQuestionNumber()));
		}
	}
}

void SheetLayoutEditor::applyQuestionEditor() {

	//Find the number of selected questions
	size_t numQuestions = 0;
	QuestionLayout* focusedQuestion_ptr = nullptr;
	for(auto treeItem_ptr : ui->layoutTree->selectedItems()) {
		if(treeItem_ptr->type() == static_cast<int>(TreeItemType::QUESTION_LAYOUT)) {
			QuestionLayout* question_ptr = dynamic_cast<QuestionLayout*>(findLayoutElement(treeItem_ptr));
			if(question_ptr != nullptr) {
				focusedQuestion_ptr = question_ptr;
				numQuestions++;
			}
		}
	}

	//There isn't much of a reason to want to change multiple questions at once, so only update questions if there is only one selected

	if(numQuestions == 1) {
		//If a question number has been specified set it on the selected question, otherwise reset the question number to its default value.
		if(ui->questionNumberEdit->text().isEmpty()) {
			focusedQuestion_ptr->setQuestionNumber(-1);
		} else {
			focusedQuestion_ptr->setQuestionNumber(ui->questionNumberEdit->text().toInt());
		}
		//Make sure to tell the question's parent that its question number has changed
		if(focusedQuestion_ptr->getParent() != nullptr) {
			GroupLayout* parent_ptr = dynamic_cast<GroupLayout*>(focusedQuestion_ptr->getParent());
			if(parent_ptr != nullptr) {
				parent_ptr->refreshQuestionNumbers();
			} else {
				tlOss << "Failed to re-sort question group";
				qlog.warning(__FILE__, __LINE__, this, tlOss);
			}
		}
	}

	buildLayoutTree();
	reloadEditorImage();

}

void SheetLayoutEditor::resetQuestionEditor() {
	ui->questionNumberEdit->setText("");
}

void SheetLayoutEditor::updateGroupEditor() {
	resetGroupEditor();

	//Find the number of selected groups
	size_t numGroups = 0;
	GroupLayout* focusedGroup_ptr = nullptr;
	for(auto treeItem_ptr : ui->layoutTree->selectedItems()) {
		if(treeItem_ptr->type() == static_cast<int>(TreeItemType::QUESTION_GROUP_LAYOUT)) {
			GroupLayout* group_ptr = dynamic_cast<GroupLayout*>(findLayoutElement(treeItem_ptr));
			if(group_ptr != nullptr) {
				focusedGroup_ptr = group_ptr;
				numGroups++;
			}
		}
	}

	//If there is only one selected group, update text boxes to display group-specific information
	if(numGroups == 1) {
		ui->groupNameInput->setText(QString::fromStdString(focusedGroup_ptr->getName()));
	}
}

void SheetLayoutEditor::applyGroupEditor() {
	//Iterate overr all selected groups
	for(auto treeItem_ptr : ui->layoutTree->selectedItems()) {
		if(treeItem_ptr->type() == static_cast<int>(TreeItemType::QUESTION_GROUP_LAYOUT)) {
			GroupLayout* group_ptr = dynamic_cast<GroupLayout*>(findLayoutElement(treeItem_ptr));
			if(group_ptr != nullptr) {
				//Update properties of this group layout to reflect changes in the group editor
				group_ptr->setName(ui->groupNameInput->text().toStdString());
			}
		}
	}

	buildLayoutTree();
	reloadEditorImage();
}

void SheetLayoutEditor::resetGroupEditor() {
	ui->groupNameInput->setText("");
}

void SheetLayoutEditor::removeLayoutElements(QList<QTreeWidgetItem*>& items) {
	int status = 0;

	//Find all layout elements to be removed before removing any of them. This is done because findLayoutElement() produces undefined behavoir if the GUI is out of synch with the model, but refreshing the GUI between each delete operation is expensive
	std::vector<SheetLayoutElement*> layoutElements;
	for(const auto& item : items) {
		if(item == nullptr) {
			status = -1;
			tlOss << "Attempted to delete null tree item";
			qlog.critical(__FILE__, __LINE__, this, tlOss);
		}

		if(status >= 0) {
			SheetLayoutElement* layoutElement = findLayoutElement(item);
			if(layoutElement == nullptr) {
				status = -1;
				tlOss << "Failed to find sheet layout element to delete.";
			} else {
				layoutElements.push_back(layoutElement);
			}
		}
	}

	for(const auto& layoutElement : layoutElements) {
		if(status >= 0) {
			//Remove the sheet layout element from its parent
			SheetLayoutElement* parent = layoutElement->getParent();
			//If parent is null than the sheet layout element is either unowned or it is a side layout element. Right now removing side layout elements is not supported, so assume it is unowned
			if(parent == nullptr) {
				unownedLayoutElements.remove(layoutElement);
			} else {
				parent->removeChild(layoutElement);
			}
		}
	}
}

bool SheetLayoutEditor::isOwned(QTreeWidgetItem* item) {
	int status = 0;
	if(item == nullptr) {
		status = -1;
		tlOss << "Encountered null tree item.";
		qlog.critical(__FILE__, __LINE__, this, tlOss);
	}

	QTreeWidgetItem* parent = nullptr;
	bool isOwned = false;
	if(status >= 0) {
		parent = item->parent();
		if(parent == nullptr) {
			//If parent is item is a null pointer then this is a top level item in the layout tree and is owned by the sheet layout element
			status = 1;
			isOwned = true;
		}
	}

	if(status == 0) {
		if(parent->type() != (int)TreeItemType::UNASSIGNED_ITEM_LIST) {
			isOwned = true;
		}
	}

	return isOwned;
}

SheetLayoutElement* SheetLayoutEditor::findLayoutElement(QTreeWidgetItem* item) {
	int status = 0;

	if(item == nullptr) {
		status = -1;
	}

	SheetLayoutElement* element = nullptr;
	if(status == 0) {
		//Check if this is a top level item of the layout tree (indexOfTopLevelItem() returns -1 if it is not)
		int index = ui->layoutTree->indexOfTopLevelItem(item);
		if(index >= 0) {
			//If this is a top level item of the layout tree, then it is either a sheet layout or the unowned layout elements item (in which case this function should return a null pointer.
			if(item->type() == (int)TreeItemType::SIDE_LAYOUT) {
				element = currentLayout_.sideLayout(index);
			} else {
				element = nullptr;
			}
		} else {
			//If this is not a top level item, request it from its parent
			QTreeWidgetItem* parentItem = item->parent();
			index = parentItem->indexOfChild(item);
			SheetLayoutElement* parentElement = findLayoutElement(parentItem);
			if(parentElement == nullptr) {
				//If the parent element does not exist then this element is unowned and can be found in the unowned layout elements list
				element = unownedLayoutElements.elementAt(index);
			} else {
				element = parentElement->childAt(index);
			}
		}
	}

	return element;
}
