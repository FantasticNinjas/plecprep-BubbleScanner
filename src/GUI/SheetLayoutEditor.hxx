#pragma once

#include <map>

#include <QDialog>
#include <QTreeWidgetItem>

#include "ui_SheetLayoutEditor.h"

#include "ScanSheetLayout.hxx"
#include "SheetScan.hxx"
#include "LayoutElementContainer.hxx"

enum class TreeItemType : int {
	UNKNOWN = QTreeWidgetItem::UserType,
	SIDE_LAYOUT,
	QUESTION_GROUP_LAYOUT,
	QUESTION_LAYOUT,
	BUBBLE_LAYOUT,
	UNASSIGNED_ITEM_LIST
};

class SheetLayoutEditor : public QDialog {
	Q_OBJECT

public:
	SheetLayoutEditor(QWidget *parent = Q_NULLPTR);
	~SheetLayoutEditor();

private slots:
	void on_layoutChooser_activated(const QString &text);

	void on_layoutTree_itemSelectionChanged();

	void on_zoomOutButton_clicked();
	void on_zoomInButton_clicked();

	void on_alignBackgroundButton_clicked();
	void on_recognizeCirclesButton_clicked();

	void on_bubbleTextEdit_editingFinished();
	void on_bubbleXEdit_editingFinished();
	void on_bubbleYEdit_editingFinished();
	void on_bubbleWidthEdit_editingFinished();
	void on_bubbleHeightEdit_editingFinished();

	void on_addBubble_clicked();
	void on_deleteBubbles_clicked();
	void on_questionFromBubbles_clicked();

	void on_questionNumberEdit_editingFinished();

	void on_groupFromQuestions_clicked();

	void on_groupNameInput_editingFinished();

	void on_groupToSide_clicked();

	void on_boxSelectActivate_clicked();

	void on_saveButton_clicked();

private:
	Ui::SheetLayoutEditor *ui;
	//Map of layout titles to layout filenames. Used by openLayout to get the file from which to load a layout by its title.
	std::map<std::string, std::string> layouts_{};


	//The current sheet layout being modified
	EasyGrade::ScanSheetLayout currentLayout_{};

	//Stores any layout elements that have been created using the GUI but have not yet been added to a parent
	EasyGrade::LayoutElementContainer unownedLayoutElements{};

	//The reference image showed in the layout editor
	SheetScan editorImage_{};
	//The zoom level of the editor image
	float editorImageScale_{1.0};

	///
	/// <summary> Retreive the list of layouts from the filesystem and display them in the leyout picker combobox. </summary>
	///
	/// <returns> Integer status code; negative means an error occured, non-negative means no error occured. </returns>
	///
	/// <note> In addition to updating the GUI, this method also updates the internal map of layout titles to filenames used when opening a layout </note>
	///
	int reloadLayoutList();

	///
	/// <summary> Open the layout currently selected in the layout chooser. If "new sheet layout" is selected, create a new sheet layout and then open it. </summary>
	///
	void openSelectedLayout();

	///
	/// <summary> Open a layout in the editor by name. This method must be called after reloadLayoutList(). </summary>
	///
	/// <param name="layoutTitle"> The name of the layout to open. This must be one of the layouts retrieved by reloadLayoutList(). </param>
	///
	int openLayout(const std::string& layoutTitle);

	///
	/// <summary> Build the layout tree display based on the currently open layout </summary>
	///
	int buildLayoutTree();

	///
	/// <summary> Get a tree item representing a sheet layout element and all of its children. Used by buildLayoutTree.</summary>
	/// <param name="layoutElement"> layoutElement The layout element to generate a tree item for </param>
	/// <param name="parent"> The tree item that should be this item's parent. If this parameter is omitted the item will have no parent.
	///
	QTreeWidgetItem* buildTreeWidget(EasyGrade::SheetLayoutElement* layoutElement, QTreeWidgetItem* parent = nullptr);

	///
	/// <summary> Open an image file as the editor background image and display it in the editor. </summary>
	///
	/// <param name="filename"> The filename of the image to load. Note that this must be a fully filename (including its path). The filename returned by SheetLayout::backgroundImageFilename() will not suffice. </param>
	///
	int openEditorImage(const std::string& filename);


	void annotateEditorImage();

	void boxSelection(const EasyGrade::Rectangle& selectionBox);

	///
	/// <summary> Reloads the editor background image from the SheetScan in memory. This will make the GUI reflect any changes / image processing that have been applied to the SheetScan. </summary>
	///
	int reloadEditorImage();

	///
	/// <summary> Reset the scale of the editor image to 100% </summary>
	///
	int resetEditorImageScale();

	///
	/// <summary> Zoom the editor image in or out. Calling this method also corrects for any mismatch between the size of the pixmap and the size of the imageLabel that can occur when the size of the editor image changed </summary>
	///
	/// <param name="amount"> The amount to scale the image. This is relative to the current scale (i.e. 1.2 makes the image 20% larger than it currently is). </param>
	///
	int zoomEditorImage(float amount = 1.0f);

	int reloadAlgorithmList();

	void updateBubbleEditor();
	void applyBubbleEditor();
	void resetbubbleEditor();

	void updateQuestionEditor();
	void applyQuestionEditor();
	void resetQuestionEditor();

	void updateGroupEditor();
	void applyGroupEditor();
	void resetGroupEditor();

	///
	/// <summary> Removes a sheet layout element. This method does not just unassign the layout element from its parent, but actually completely deletes it. </summary>
	///
	/// <param name="item"> The layout tree item representing the layout element to be removed </param>
	///
	/// <returns> Non-negative if successful, negative if an error occured. </returns>
	///
	void removeLayoutElements(QList<QTreeWidgetItem*>& items);

	///
	/// <summary> Check whether an item in the layout tree display is owned by a parent layout element. </summary>
	///
	/// <param name="item"> The item to check </param>
	///
	/// <returns> True if the layout tree item is has a parent layout element, false if it is in the unassigned layout elements list </returns>
	///
	bool isOwned(QTreeWidgetItem *item);

	EasyGrade::SheetLayoutElement* findLayoutElement(QTreeWidgetItem* item);
};