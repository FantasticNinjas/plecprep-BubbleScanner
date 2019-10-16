#include "ScantronReader.hxx"
#include "DetectionParams.hxx"
#include "TextLogging.hxx"
#include "SheetLayout.hxx"
#include <QtWidgets/QApplication>
#include "Reader.hxx"
#include <vector>
#include <stdio.h>
#include <qdebug.h>
#include <sstream>

int main(int argc, char *argv[])
{

	//TextLogging::setIsDebugVerbosityEnabledDefault(true);
	
	std::ostringstream tlOss;
	TextLogging tlog;
	
	DetectionParams temp;
	if(temp.load("./config/detection-params.xml", "Basic Threshold-Fraction Filter") >= 0) {
		tlOss << "Successfully loaded detection params.";
		tlog.info(__FILE__, __LINE__, tlOss);

		std::ostringstream debugOss;
		debugOss << temp;
		qDebug() << debugOss.str().c_str();

		for(const auto& iter : temp.getParamTable()) {
			debugOss.str("");
			debugOss << "is " << iter.first << " a float? " << (temp.isFloat(iter.first) ? "true." : "false.") << std::endl;
			debugOss << "is " << iter.first << " an int? " << (temp.isInt(iter.first) ? "true." : "false.") << std::endl;
			qDebug() << debugOss.str().c_str();
		}

	} else {
		tlOss << "Failed to load detection params. See log.";
		tlog.critical(__FILE__, __LINE__, tlOss);
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
