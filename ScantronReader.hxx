#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ScantronReader.h"
#include "SheetLayoutEditor.hxx"
#include <memory>

class ScantronReader : public QMainWindow
{
	Q_OBJECT

public:
	ScantronReader(QWidget *parent = Q_NULLPTR);

private slots:
	void on_actionOpen_Layout_Editor_triggered();


private:
	Ui::ScantronReaderClass ui;
	std::unique_ptr<SheetLayoutEditor> sheetLayoutEditor;
};
