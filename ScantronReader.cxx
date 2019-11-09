#include "ScantronReader.hxx"
#include "SheetLayoutEditor.hxx"

ScantronReader::ScantronReader(QWidget *parent) : QMainWindow(parent) {

	//Set up user interface
	ui.setupUi(this);
	ui.statusBar->addPermanentWidget(ui.label, 1);
	ui.statusBar->addPermanentWidget(ui.progressBar, 2);
	ui.progressBar->hide();
}


void ScantronReader::on_actionOpen_Layout_Editor_triggered() {
	sheetLayoutEditor = std::make_unique<SheetLayoutEditor>(this);
	sheetLayoutEditor->show();
}
