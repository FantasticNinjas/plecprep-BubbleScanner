#include <QDir>
#include <QMessageBox>
#include <QScrollBar>
#include <QDebug>

#include <sstream>

#include "SheetLayoutEditor.hxx"
#include "ui_SheetLayoutEditor.h"

#include "FilenameOracle.hxx"
#include "QtLogging.hxx"
#include "SheetLayoutTreeBuilder.hxx"

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
	for(const auto& treeItem : ui->layoutTree->selectedItems()) {
		if(treeItem->type() == (int)TreeItemType::SIDE_LAYOUT) {
			struct SideLayout* layout = findSideLayout(treeItem);
			if(layout == nullptr) {
				qDebug() << "Invalid side";
			} else {
				qDebug() << "Side number " << layout->sideNumber_;
			}
		} else if(treeItem->type() == (int)TreeItemType::QUESTION_GROUP_LAYOUT) {
			struct QuestionGroupLayout* layout = findQuestionGroupLayout(treeItem);
			if(layout == nullptr) {
				qDebug() << "Invalid question group";
			} else {
				qDebug() << "Question Group \"" << layout->name_.c_str() << "\"";
			}
		} else if(treeItem->type() == (int)TreeItemType::QUESTION_LAYOUT) {
			struct QuestionLayout* layout = findQuestionLayout(treeItem);
			if(layout == nullptr) {
				qDebug() << "Invalid question";
			} else {
				qDebug() << "Question \"" << layout->questionNumber_ << "\"";
			}
		} else if(treeItem->type() == (int)TreeItemType::BUBBLE_LAYOUT) {
			struct BubbleLayout* layout = findBubbleLayout(treeItem);
			if(layout == nullptr) {
				qDebug() << "Invalid bubble";
			} else {
				qDebug() << "Bubble \"" << layout->answer_.c_str() << "\"";
			}
		}
	}
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
	if(algorithmName.empty()) {
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
	static int groupNumber;
	QuestionGroupLayout groupLayout;

	std::ostringstream groupName;
	groupName << "Question Group " << ++groupNumber;

	groupLayout.name_ = groupName.str();
	unassignedLayoutElements_.add(groupLayout);
	displayLayoutTree();
}

int SheetLayoutEditor::reloadLayoutList() {
	//Get a list of all of the readable xml files in the sheet layout directory
	int status = 0;

	//Remove any existing items in the layout picker
	ui->layoutChooser->clear();

	QDir sheetLayoutsDirectory(QString::fromStdString(FilenameOracle::getLayoutDirectoryFilename()), "*.xml");
	QStringList layoutFilenames = sheetLayoutsDirectory.entryList();

	tlOss << "Loaded " << layoutFilenames.size() << " sheet layout files.";
	qlog.debug(__FILE__, __LINE__, this, tlOss);

	//Get the title of each sheet layout in the list and add it to the drop down menu as well as the list of sheet layouts (SheetLayoutEditor::layouts);
	for(const auto& filename : layoutFilenames) {
		//Note: The list of items provided by QDir does not include paths, only the name of the file itself. As such, it is necessary 
		//to add the directory name manually to each before passing it to getLayoutTitle
		std::string qualifiedFilename = FilenameOracle::getLayoutDirectoryFilename() + filename.toStdString();
		std::string layoutTitle = SheetLayout::getLayoutTitle(qualifiedFilename);

		if(layoutTitle.empty()) {
			tlOss << "Failed to get sheet layout title for layout file \"" << qualifiedFilename << "\"";
			qlog.warning(__FILE__, __LINE__, this, tlOss);
		} else {
			ui->layoutChooser->addItem(QString::fromStdString(layoutTitle));
			layouts_[layoutTitle] = qualifiedFilename;
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

int SheetLayoutEditor::openLayout(const std::string & layoutTitle) {
	int status = 0;

	//Check that the layout to load is in the list of layouts.
	if(layouts_.find(layoutTitle) == layouts_.end()) {
		status = -1;
		tlOss << "Unrecognized sheet layout title \"" << layoutTitle << "\".";
		qlog.critical(__FILE__, __LINE__, this, tlOss);
	}

	//Load the sheet layout
	if(status >= 0) {
		if(currentLayout_.load(layouts_[layoutTitle]) < 0) {
			status = -1;
			tlOss << "Failed to load sheet layout \"" << layoutTitle << "\" from \"" << layouts_[layoutTitle] << "\".";
			qlog.critical(__FILE__, __LINE__, this, tlOss);
		}
	}

	//Populate the layout tree widget
	if(status >= 0) {
		status = displayLayoutTree();
	}


	//Load the editor background image specified in the layout file
	if(status >= 0) {
		status = editorImage_.load(FilenameOracle::getLayoutDirectoryFilename() + FilenameOracle::getImageDirectory() + currentLayout_.getBackgroundImageFilename());
	}

	//Display the editor background image.
	if(status >= 0) {
		status = reloadEditorImage();
	}

	return status;
}

int SheetLayoutEditor::displayLayoutTree() {
	int status = 0;

	//Clear out any items already in the tree
	ui->layoutTree->clear();

	//List to hold top level tree elements (the sheet sides)
	QList<QTreeWidgetItem *> sheetSideItems;
	for(const auto& sideLayout : currentLayout_.getSideLayouts()) {
		//Generate subtrees for each of the side layouts in the current layout. 
		QTreeWidgetItem* currentSideItem = LayoutTreeBuilder::generateTreeItem(sideLayout);
		sheetSideItems.push_back(currentSideItem);
	}

	//Create tree items to represent unassigned layout elements
	QTreeWidgetItem* unassignedElementItems = new QTreeWidgetItem((QTreeWidgetItem*)nullptr, QStringList("Unassigned Elements"), (int)TreeItemType::UNASSIGNED_ITEM_LIST);
	unassignedElementItems->addChildren(unassignedLayoutElements_.treeWidgetItem());

	//Add the tree of items to the layout tree display
	ui->layoutTree->addTopLevelItems(sheetSideItems);

	//Add unassigned layout items to layout tree display
	ui->layoutTree->addTopLevelItem(unassignedElementItems);

	return status;
}

int SheetLayoutEditor::openEditorImage(const std::string& filename) {
	int status = 0;

	//Load the image specified by filename
	if(editorImage_.load(filename) < 0) {
		status = -1;
		tlOss << "Unable to open image file \"" << filename << "\", it may be missing or corrupt.";
		qlog.critical(__FILE__, __LINE__, this, tlOss);
	}

	if(status >= 0) {
		zoomEditorImage(1);
	}

	//Display the image in the editor
	if(status >= 0 && reloadEditorImage() < 0) {
		status = -1;
		tlOss << "Image loaded from \"" << filename << "\" (associated with this sheet layout) appears to be empty.";
		qlog.warning(__FILE__, __LINE__, this, tlOss);
	}


	return status;
}

int SheetLayoutEditor::reloadEditorImage() {
	int status = 0;

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

SideLayout* SheetLayoutEditor::findSideLayout(QTreeWidgetItem*item) {
	int status = 0;

	//Check that provided widget item exists and represents a side layout

	if(item == nullptr || item->type() != (int)TreeItemType::SIDE_LAYOUT) {
		status = -1;
		tlOss << "Selected item does not represent a side layout but was interpreted as one";
		qlog.critical(__FILE__, __LINE__, this, tlOss);
	}

	//Get the index of the provided widget item.
	//Note that side layouts should always be a top level item.
	int sideNumber;
	if(status >= 0) {
		sideNumber = ui->layoutTree->indexOfTopLevelItem(item);

		if(sideNumber < 0) {
			status = -1;
			tlOss << "Selected side layout is not a top level item.";
			qlog.critical(__FILE__, __LINE__, this, tlOss);
		}
	}

	//Check that sideNumber is a valid index in the SheetLayout. This check should never fail, but it's worth doing in case the GUI has somehow gotten out of sync with the currentLayout
	if(status >= 0) {
		if(sideNumber >= currentLayout_.numSideLayouts()) {
			status = -1;
			tlOss << "GUI requested side layout that does not exist (index " << sideNumber << " of " << currentLayout_.numSideLayouts() << ") The user interface may be out of sync with the internal state of the program.";
			qlog.critical(__FILE__, __LINE__, this, tlOss);
		}
	}

	//Retrieve the requested side layout
	struct SideLayout* sideLayout = nullptr;
	if(status >= 0) {
		sideLayout = &currentLayout_.getSideLayouts()[sideNumber];
	}

	return sideLayout;
}

struct QuestionGroupLayout* SheetLayoutEditor::findQuestionGroupLayout(QTreeWidgetItem* item) {

	int status = 0;

	//Check that the provided tree widget item represents a question group
	if(item == nullptr || item->type() != (int)TreeItemType::QUESTION_GROUP_LAYOUT) {
		status = -1;
		tlOss << "Selected item does not represent a question group layout but was interpreted as one";
		qlog.critical(__FILE__, __LINE__, this, tlOss);
	}

	//Check that the provided tree widget item is in the layout tree and is not a top level item
	QTreeWidgetItem* parent  = nullptr;
	if(status >= 0) {
		parent = item->parent();
		if(parent == nullptr) {
			status = -1;
			tlOss << "Attempted to access question group item that is not present in the layout tree or is erroniously a top level item.";
			qlog.critical(__FILE__, __LINE__, this, tlOss);
		}
	}
	
	//Find the QuestionGroupLayout represented by the tree widget item
	struct QuestionGroupLayout* questionGroup = nullptr;
	if(status >= 0) {
		//Find whetehr the question group is part of a side layout or on the list of unassigned items
		int parentType = parent->type();
		int childIndex = parent->indexOfChild(item);
		//Parent of a question group should always be a side layout or the unassigned elements item
		if(parentType == (int)TreeItemType::SIDE_LAYOUT) {
			//If the parent item represents a side layout, find the side layout it represents.
			struct SideLayout* parentSideLayout = findSideLayout(parent);
			if(parentSideLayout == nullptr) {
				status = -1;
				tlOss << "Unable to find side layout containing the selected question group.";
				qlog.critical(__FILE__, __LINE__, this, tlOss);
			}

			//Check that childIndex is a valid question group index
			if(status >= 0) {
				if(childIndex >= parentSideLayout->getNumGroups()) {
					status = -1;
					tlOss << "GUI requested question group layout that does not exist (index " << childIndex << " of " << parentSideLayout->getNumGroups();
					qlog.critical(__FILE__, __LINE__, this, tlOss);
				}
			}

			//Get the question group from the side layout that owns it
			if(status >= 0) {
				questionGroup = &parentSideLayout->questionGroups_[childIndex];
			}
		} else if(parentType == (int)TreeItemType::UNASSIGNED_ITEM_LIST) {
			//The question group has not yet been added to the layout tree and is instead part of the "unassigned items" list.
			questionGroup = unassignedLayoutElements_.getQuestionGroupLayout(childIndex);
			if(questionGroup == nullptr) {
				tlOss << "Failed to retrieve question group associated with selected item";
				qlog.critical(__FILE__, __LINE__, this, tlOss);
			}

		} else {
			status = -1;
			tlOss << "Found question group layout tree item in an invalid location.";
			qlog.critical(__FILE__, __LINE__, this, tlOss);
		}
	}

	return questionGroup;
}

QuestionLayout * SheetLayoutEditor::findQuestionLayout(QTreeWidgetItem * item) {

	int status = 0;

	//Check that the provided tree widget item represents a question
	if(item == nullptr || item->type() != (int)TreeItemType::QUESTION_LAYOUT) {
		status = -1;
		tlOss << "Selected item does not represent a question layout but was interpreted as one";
		qlog.critical(__FILE__, __LINE__, this, tlOss);
	}

	//Check that the provided tree widget item is in the layout tree and is not a top level item
	QTreeWidgetItem* parent = nullptr;
	if(status >= 0) {
		parent = item->parent();
		if(parent == nullptr) {
			status = -1;
			tlOss << "Attempted to access question item that is not present in the layout tree or is erroniously a top level item.";
			qlog.critical(__FILE__, __LINE__, this, tlOss);
		}
	}

	//Find the QuestionLayout represented by the tree widget item
	struct QuestionLayout* question = nullptr;
	if(status >= 0) {
		//Find whetehr the question is part of a side layout or on the list of unassigned items
		int parentType = parent->type();
		int childIndex = parent->indexOfChild(item);
		//Parent of a question should always be a question group or the unassigned elements item
		if(parentType == (int)TreeItemType::QUESTION_GROUP_LAYOUT) {
			//If the parent item represents a question group layout, find the question group layout it represents.
			struct QuestionGroupLayout* parentGroupLayout = findQuestionGroupLayout(parent);
			if(parentGroupLayout == nullptr) {
				status = -1;
				tlOss << "Unable to find question group containing the selected question.";
				qlog.critical(__FILE__, __LINE__, this, tlOss);
			}

			//Check that childIndex is a valid question group index
			if(status >= 0) {
				if(childIndex >= parentGroupLayout->getNumQuestions()) {
					status = -1;
					tlOss << "GUI requested question layout that does not exist (index " << childIndex << " of " << parentGroupLayout->getNumQuestions();
					qlog.critical(__FILE__, __LINE__, this, tlOss);
				}
			}

			//Get the question group from the side layout that owns it
			if(status >= 0) {
				question = &parentGroupLayout->questions_[childIndex];
			}
		} else if(parentType == (int)TreeItemType::UNASSIGNED_ITEM_LIST) {
			//The question group has not yet been added to the layout tree and is instead part of the "unassigned items" list.
			question = unassignedLayoutElements_.getQuestionLayout(childIndex);
			if(question == nullptr) {
				tlOss << "Failed to retrieve question associated with selected item";
				qlog.critical(__FILE__, __LINE__, this, tlOss);
			}

		} else {
			status = -1;
			tlOss << "Found question layout tree item in an invalid location.";
			qlog.critical(__FILE__, __LINE__, this, tlOss);
		}
	}

	return question;
}

BubbleLayout * SheetLayoutEditor::findBubbleLayout(QTreeWidgetItem * item) {

	int status = 0;

	//Check that the provided tree widget item represents a bubble
	if(item == nullptr || item->type() != (int)TreeItemType::BUBBLE_LAYOUT) {
		status = -1;
		tlOss << "Selected item does not represent a bubble layout but was interpreted as one";
		qlog.critical(__FILE__, __LINE__, this, tlOss);
	}

	//Check that the provided tree widget item is in the layout tree and is not a top level item
	QTreeWidgetItem* parent = nullptr;
	if(status >= 0) {
		parent = item->parent();
		if(parent == nullptr) {
			status = -1;
			tlOss << "Attempted to access bubble item that is not present in the layout tree or is erroniously a top level item.";
			qlog.critical(__FILE__, __LINE__, this, tlOss);
		}
	}

	//Find the BubbleLayout represented by the tree widget item
	struct BubbleLayout* bubble = nullptr;
	if(status >= 0) {
		//Find whether the bubble is part of a side layout or on the list of unassigned items
		int parentType = parent->type();
		int childIndex = parent->indexOfChild(item);
		//Parent of a bubble should always be a question or the unassigned elements item
		if(parentType == (int)TreeItemType::QUESTION_LAYOUT) {
			//If the parent item represents a question layout, find the question layout it represents.
			struct QuestionLayout* parentQuestionLayout = findQuestionLayout(parent);
			if(parentQuestionLayout == nullptr) {
				status = -1;
				tlOss << "Unable to find question containing the selected bubble.";
				qlog.critical(__FILE__, __LINE__, this, tlOss);
			}

			//Check that childIndex is a valid question group index
			if(status >= 0) {
				if(childIndex >= parentQuestionLayout->bubbles_.size()) {
					status = -1;
					tlOss << "GUI requested bubbleLayout that does not exist (index " << childIndex << " of " << parentQuestionLayout->bubbles_.size();
					qlog.critical(__FILE__, __LINE__, this, tlOss);
				}
			}

			//Get the question group from the side layout that owns it
			if(status >= 0) {
				bubble = &parentQuestionLayout->bubbles_[childIndex];
			}
		} else if(parentType == (int)TreeItemType::UNASSIGNED_ITEM_LIST) {
			//The question group has not yet been added to the layout tree and is instead part of the "unassigned items" list.
			bubble = unassignedLayoutElements_.getBubbleLayout(childIndex);
			if(bubble == nullptr) {
				tlOss << "Failed to retrieve bubble associated with selected item";
				qlog.critical(__FILE__, __LINE__, this, tlOss);
			}

		} else {
			status = -1;
			tlOss << "Found bubble layout tree item in an invalid location.";
			qlog.critical(__FILE__, __LINE__, this, tlOss);
		}
	}

	return bubble;
}
