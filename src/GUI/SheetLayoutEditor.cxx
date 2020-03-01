#include <QDir>
#include <QMessageBox>
#include <QScrollBar>
#include <QDebug>

#include <sstream>

#include "SheetLayoutEditor.hxx"
#include "ui_SheetLayoutEditor.h"

#include "FilenameOracle.hxx"
#include "QtLogging.hxx"
#include "FileUtil.hxx"

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
	focusedSideLayout = nullptr;
	focusedGroupLayout = nullptr;
	focusedQuestionLayout = nullptr;
	focusedBubbleLayout = nullptr;

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
	QList<QTreeWidgetItem*> itemsToDelete;
	for(const auto& treeItem : ui->layoutTree->selectedItems()) {
		if(treeItem->type() == static_cast<int>(TreeItemType::BUBBLE_LAYOUT)) {
			itemsToDelete.append(treeItem);
		}
	}

	removeLayoutElements(itemsToDelete);
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
		QFile currentLayoutFile(fileInfo.absoluteFilePath());
		currentLayoutFile.open(QIODevice::ReadOnly | QIODevice::Text);
		
		//Read the sheet layout file into a scan sheet layout
		ScanSheetLayout currentLayout;
		if(currentLayout.readXml(FileUtil::getInputStream(currentLayoutFile)) < 0) {
			tlOss << "Failed to read sheet layout file \"" << fileInfo.fileName().toStdString() << "\"";
			qlog.warning(__FILE__, __LINE__, this, tlOss);
		} else {
			//Add this sheet layout to the layout picker box
			std::string layoutTitle = currentLayout.getTitle();
			ui->layoutChooser->addItem(QString::fromStdString(layoutTitle));
			layouts_[layoutTitle] = fileInfo.absoluteFilePath().toStdString();
		}
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
		QFile sheetLayoutFile(QString::fromStdString(layouts_[layoutTitle]));
		sheetLayoutFile.open(QIODevice::ReadOnly | QIODevice::Text);
		if(currentLayout_.readXml(FileUtil::getInputStream(sheetLayoutFile)) < 0) {
			status = -1;
			tlOss << "Failed to load sheet layout \"" << layoutTitle << "\" from \"" << layouts_[layoutTitle] << "\"";
			qlog.critical(__FILE__, __LINE__, this, tlOss);
		}
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
	int status = 0;

	if(focusedBubbleLayout == nullptr) {
		tlOss << "Not updating bubble editor because no bubble is selected.";
		qlog.debug(__FILE__, __LINE__, this, tlOss);
		resetbubbleEditor();
		status = 1;
	}

	if(status == 0) {
		//Set bubble text
		ui->bubbleTextEdit->setText(QString::fromStdString(focusedBubbleLayout->getAnswer()));
		//Set bubble x coordinate
		std::ostringstream xCoordStream;
		xCoordStream << focusedBubbleLayout->getLocation()[0];
		ui->bubbleXEdit->setText(QString::fromStdString(xCoordStream.str()));
		//Set bubble y coordinate
		std::ostringstream yCoordStream;
		yCoordStream << focusedBubbleLayout->getLocation()[1];
		ui->bubbleYEdit->setText(QString::fromStdString(yCoordStream.str()));
		//Set bubble radius
		std::ostringstream radiusStream;
		radiusStream << focusedBubbleLayout->getLocation()[2];
		ui->bubbleRadiusEdit->setText(QString::fromStdString(radiusStream.str()));

		//Set box selection bounds to boundingbox
		cv::Rect2f boundingBox = focusedBubbleLayout->boundingBox();
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
	int status = 0;

	if(focusedBubbleLayout == nullptr) {
		tlOss << "Not updating bubble editor because no bubble is selected.";
		qlog.debug(__FILE__, __LINE__, this, tlOss);
		resetbubbleEditor();
		status = 1;
	}

	if(status == 0) {
		//Set fields on focused bubble
		focusedBubbleLayout->setAnswer(ui->bubbleTextEdit->text().toStdString());
		focusedBubbleLayout->setCenterX(ui->bubbleXEdit->text().toFloat());
		focusedBubbleLayout->setCenterY(ui->bubbleYEdit->text().toFloat());
		focusedBubbleLayout->setRadius(ui->bubbleRadiusEdit->text().toFloat());

		buildLayoutTree();
		reloadEditorImage();
	}
}

void SheetLayoutEditor::resetbubbleEditor() {
	ui->bubbleTextEdit->setText("");
	ui->bubbleXEdit->setText("");
	ui->bubbleYEdit->setText("");
	ui->bubbleRadiusEdit->setText("");
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
