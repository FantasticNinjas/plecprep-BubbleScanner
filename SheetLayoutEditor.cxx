#include <QDir>
#include <QMessageBox>
#include <QScrollBar>
#include <QDebug>

#include <sstream>

#include "SheetLayoutEditor.hxx"
#include "ui_SheetLayoutEditor.h"

#include "FilenameOracle.hxx"
#include "TextLogging.hxx"

namespace {
	std::ostringstream tlOss;
	TextLogging tlog;
}

SheetLayoutEditor::SheetLayoutEditor(QWidget *parent) : QDialog(parent) {
	ui = new Ui::SheetLayoutEditor();
	ui->setupUi(this);
	ui->imageScrollArea->setWidget(ui->imageLabel);
	ui->imageLabel->setScaledContents(true);
	if(reloadLayoutList() < 0) {
		QMessageBox::warning(this, "Warning", "An error occured while loading the list of sheet layouts. Check the log file for details.");
	}
}

SheetLayoutEditor::~SheetLayoutEditor() {
	delete ui;
}

void SheetLayoutEditor::on_layoutChooser_activated(const QString &text) {
	if(ui->layoutChooser->currentIndex() < ui->layoutChooser->count() - 1) {
		if(openLayout(text.toStdString()) < 0) {
			QMessageBox::critical(this, "Error", "Failed to load sheet layout \"" + text + "\"");
		}
	}
}

void SheetLayoutEditor::on_zoomOutButton_clicked() {
	zoomEditorImage(0.8);
}

void SheetLayoutEditor::on_zoomInButton_clicked() {
	zoomEditorImage(1.25);
}

int SheetLayoutEditor::reloadLayoutList() {
	//Get a list of all of the readable xml files in the sheet layout directory
	int status = 0;

	QDir sheetLayoutsDirectory(QString::fromStdString(FilenameOracle::getLayoutDirectoryFilename()), "*.xml");
	QStringList layoutFilenames = sheetLayoutsDirectory.entryList();

	tlOss << "Loaded " << layoutFilenames.size() << " sheet layout files.";
	tlog.debug(__FILE__, __LINE__, tlOss);

	//Get the title of each sheet layout in the list and add it to the drop down menu as well as the list of sheet layouts (SheetLayoutEditor::layouts);
	for(const auto& filename : layoutFilenames) {
		//Note: The list of items provided by QDir does not include paths, only the name of the file itself. As such, it is necessary 
		//to add the directory name manually to each before passing it to getLayoutTitle
		std::string qualifiedFilename = FilenameOracle::getLayoutDirectoryFilename() + filename.toStdString();
		std::string layoutTitle = SheetLayout::getLayoutTitle(qualifiedFilename);

		if(layoutTitle.empty()) {
			tlOss << "Failed to get sheet layout title for layout file \"" << qualifiedFilename << "\"";
			tlog.warning(__FILE__, __LINE__, tlOss);
		} else {
			ui->layoutChooser->addItem(QString::fromStdString(layoutTitle));
			layouts[layoutTitle] = qualifiedFilename;
		}
	}

	//Add new sheet layout option to the list.
	ui->layoutChooser->addItem("<Create New Sheet Layout>");

	return status;
}

int SheetLayoutEditor::openLayout(const std::string & layoutTitle) {
	int status = 0;

	//Check that the layout to load is in the list of layouts.
	if(layouts.find(layoutTitle) == layouts.end()) {
		status = -1;
		tlOss << "Unrecognized sheet layout title \"" << layoutTitle << "\".";
		tlog.critical(__FILE__, __LINE__, tlOss);
	}

	//Load the sheet layout
	if(status >= 0) {
		if(currentLayout.load(layouts[layoutTitle]) < 0) {
			status = -1;
			tlOss << "Failed to load sheet layout \"" << layoutTitle << "\" from \"" << layouts[layoutTitle] << "\".";
			tlog.critical(__FILE__, __LINE__, tlOss);
		}
	}

	//Populate the layout tree widget
	if(status >= 0) {
		status = displayLayoutTree();
	}


	//Load the editor background image specified in the layout file
	if(status >= 0) {
		status = editorImage.load(FilenameOracle::getLayoutDirectoryFilename() + FilenameOracle::getImageDirectory() + currentLayout.getBackgroundImageFilename());
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
	for(const auto& sideLayout : currentLayout.getSideLayouts()) {
		//Construct the display name for the current side layout
		std::ostringstream sideLayoutName;
		sideLayoutName << "Side " << sideLayout.sideNumber_ + 1 << " Layout";

		//Create tree item for the current side layout
		QTreeWidgetItem *currentSideItem = new QTreeWidgetItem((QTreeWidget*)nullptr, QStringList(QString::fromStdString(sideLayoutName.str())), (int)TreeItemType::SIDE_LAYOUT);

		//Create tree items for question groups and add them to the layout
		for(const auto& questionGroupLayout : sideLayout.questionGroups_) {
			//Add a tree item for the current question group
			QTreeWidgetItem *currentGroupItem = new QTreeWidgetItem(currentSideItem, QStringList(QString::fromStdString(questionGroupLayout.name_)), (int)TreeItemType::QUESTION_GROUP_LAYOUT);

			//Create tree items for questions
			for(const auto& questionLayout : questionGroupLayout.questions_) {
				std::ostringstream questionName;
				questionName << "Question " << questionLayout.questionNumber_ + 1;
				QTreeWidgetItem *currentQuestionItem = new QTreeWidgetItem(currentGroupItem, QStringList(QString::fromStdString(questionName.str())), (int)TreeItemType::QUESTION_LAYOUT);

				//Create tree items for each individual bubble
				for(const auto& bubbleLayout : questionLayout.bubbles_) {
					std::ostringstream bubbleName;
					bubbleName << "\"" << bubbleLayout.answer_ << "\" Bubble";
					QTreeWidgetItem *currentBubbleItem = new QTreeWidgetItem(currentQuestionItem, QStringList(QString::fromStdString(bubbleName.str())), (int)TreeItemType::QUESTION_LAYOUT);
				}
			}
		}

		//Add sheet layout to list
		sheetSideItems.push_back(currentSideItem);
	}

	//Add the tree of items to the layout tree display
	ui->layoutTree->addTopLevelItems(sheetSideItems);

	return status;
}

int SheetLayoutEditor::openEditorImage(const std::string& filename) {
	int status = 0;

	//Load the image specified by filename
	if(editorImage.load(filename) < 0) {
		status = -1;
		tlOss << "Unable to open image file \"" << filename << "\", it may be missing or corrupt.";
		QMessageBox::critical(this, "Error", QString::fromStdString(tlOss.str()));
		tlog.critical(__FILE__, __LINE__, tlOss);
	}

	if(status >= 0) {
		zoomEditorImage(1);
	}

	//Display the image in the editor
	if(status >= 0 && reloadEditorImage() < 0) {
		status = -1;
		tlOss << "Image loaded from \"" << filename << "\" (associated with this sheet layout) appears to be empty.";
		QMessageBox::warning(this, "Warning", QString::fromStdString(tlOss.str()));
		tlog.warning(__FILE__, __LINE__, tlOss);
	}


	return status;
}

int SheetLayoutEditor::reloadEditorImage() {
	int status = 0;

	//Get the pixmap to display from the SheetScan.
	QPixmap editorPixmap = editorImage.getAnnotatedPixmap();
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
	return zoomEditorImage(1.0f / editorImageScale);
}

int SheetLayoutEditor::zoomEditorImage(float amount) {
	int status = 0;

	if(amount <= 0) {
		tlOss << "Invalid scale amount: " << amount;
		QMessageBox::warning(this, "Warning", QString::fromStdString(tlOss.str()));
		tlog.warning(__FILE__, __LINE__, tlOss);
	}

	if(status >= 0) {
		//Adjust the size of the image label
		editorImageScale *= amount;
		
		ui->imageLabel->setFixedSize(editorImageScale * ui->imageLabel->pixmap()->size());
		

		qDebug() << "Image size = " << ui->imageLabel->size() << " but should be " << editorImageScale * ui->imageLabel->pixmap()->size();


		//Reposition scroll bars to keep the center in the center
		ui->imageScrollArea->horizontalScrollBar()->setValue(int(amount * ui->imageScrollArea->horizontalScrollBar()->value() + ((amount - 1) * ui->imageScrollArea->horizontalScrollBar()->pageStep() / 2)));
		ui->imageScrollArea->verticalScrollBar()->setValue(int(amount * ui->imageScrollArea->verticalScrollBar()->value() + ((amount - 1) * ui->imageScrollArea->verticalScrollBar()->pageStep() / 2)));
	}

	return status;
}