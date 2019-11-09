#pragma once

#include <map>

#include <QDialog>

#include "SheetLayout.hxx"
#include "SheetScan.hxx"

namespace Ui { class SheetLayoutEditor; };

class SheetLayoutEditor : public QDialog
{
	Q_OBJECT

public:
	SheetLayoutEditor(QWidget *parent = Q_NULLPTR);
	~SheetLayoutEditor();

private slots:
	void on_layoutChooser_activated(const QString &text);
	void on_zoomOutButton_clicked();
	void on_zoomInButton_clicked();

private:
	Ui::SheetLayoutEditor *ui;
	//Map of layout titles to layout filenames. Used by openLayout to get the file from which to load a layout by its title.
	std::map<std::string, std::string> layouts{};

	SheetLayout currentLayout{};

	SheetScan editorImage{};
	float editorImageScale{1.0};

	// The layout tree display keeps track of the type of each item (using values from this enum) in order to help trace it back to a specific entity in the SheetLayout object.
	enum class TreeItemType : int {
		UNKNOWN,
		SIDE_LAYOUT,
		QUESTION_GROUP_LAYOUT,
		QUESTION_LAYOUT,
		BUBBLE_LAYOUT
	};

	///
	/// <summary> Retreive the list of layouts from the filesystem and display them in the leyout picker combobox. </summary>
	///
	/// <returns> Integer status code; negative means an error occured, non-negative means no error occured. </returns>
	///
	/// <note> In addition to updating the GUI, this method also updates the internal map of layout titles to filenames used when opening a layout </note>
	///
	int reloadLayoutList();

	///
	/// <summary> Open a layout in the editor by name. This method must be called after reloadLayoutList(). </summary>
	///
	/// <param name="layoutTitle"> The name of the layout to open. This must be one of the layouts retrieved by reloadLayoutList(). </param>
	///
	int openLayout(const std::string& layoutTitle);

	///
	/// <summary> Build the layout tree display based on the currently open layout </summary>
	///
	int displayLayoutTree();

	///
	/// <summary> Open an image file as the editor background image and display it in the editor. </summary>
	///
	/// <param name="filename"> The filename of the image to load. Note that this must be a fully filename (including its path). The filename returned by SheetLayout::backgroundImageFilename() will not suffice. </param>
	///
	int openEditorImage(const std::string& filename);

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
	/// <param> The amount to scale the image. This is relative to the current scale (i.e. 1.2 makes the image 20% larger than it currently is). </param>
	///
	int zoomEditorImage(float amount = 1.0f);
};
