#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ScantronReader.h"

class ScantronReader : public QMainWindow
{
	Q_OBJECT

public:
	ScantronReader(QWidget *parent = Q_NULLPTR);

private:
	Ui::ScantronReaderClass ui;
};
