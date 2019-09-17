#include "ScantronReader.hxx"
#include "TextLogging.hxx"
#include "SheetLayout.hxx"
#include <QtWidgets/QApplication>
#include "Reader.hxx"
#include <vector>
#include <stdio.h>
#include <qdebug.h>
#include <sstream>

using namespace cv;

int main(int argc, char *argv[])
{

	//TextLogging::setIsDebugVerbosityEnabledDefault(true);

	//std::ostringstream tlOss;
	//TextLogging tlog;

	//tlOss << "This is a test info message.";
	//tlog.info(__FILE__, __LINE__, tlOss);

	//tlOss << "This is a test warning.";
	//tlog.warning(__FILE__, __LINE__, tlOss);

	//tlOss << "This is a test critical error.";
	//tlog.critical(__FILE__, __LINE__, tlOss);

	//tlOss << "This is another test info message.";
	//tlog.info(__FILE__, __LINE__, tlOss);

	SheetLayout temp;
	if(temp.load("./templates/template_test.xml") >= 0) {
		std::ostringstream debugOss;
		debugOss << temp;
		qDebug() << debugOss.str().c_str();
	} else {
		//tlOss << "Failed to load sheet template.";
		//tlog.critical(__FILE__, __LINE__, tlOss);
	}

	//std::vector<int> compression_params;
	//compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	//compression_params.push_back(9);

	//try {
	//	imwrite("output.png", grayedColor, compression_params);
	//}
	//catch (std::runtime_error& ex) {
	//	fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
	//	return 1;
	//}
	//

	//namedWindow("Image", WINDOW_NORMAL);
	//imshow("Image", grayedColor);
	//waitKey(0);
	//destroyAllWindows();

//	QApplication a(argc, argv);
//	ScantronReader w;
//	w.show();
//	return a.exec();
	return 0;
}
