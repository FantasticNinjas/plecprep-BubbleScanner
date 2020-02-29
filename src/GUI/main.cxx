#include "ScantronReader.hxx"
#include "DetectionParams.hxx"
#include "TextLogging.hxx"
#include "SheetScan.hxx"
#include <QtWidgets/QApplication>
#include <vector>
#include <stdio.h>
#include <qdebug.h>
#include <sstream>

int main(int argc, char *argv[])
{

	TextLogging::setIsDebugVerbosityEnabledDefault(true);
	
	std::ostringstream tlOss;
	TextLogging tlog;
	

	QApplication a(argc, argv);
	ScantronReader w;
	w.show();
	return a.exec();
	return 0;
}
