#include "ScantronReader.hxx"
#include "DetectionParams.hxx"
#include "TextLogging.hxx"
#include "SheetScan.hxx"
#include "SheetLayout.hxx"
#include <QtWidgets/QApplication>
#include "Reader.hxx"
#include <vector>
#include <stdio.h>
#include <qdebug.h>
#include <sstream>

int main(int argc, char *argv[])
{

	TextLogging::setIsDebugVerbosityEnabledDefault(true);
	
	std::ostringstream tlOss;
	TextLogging tlog;
	
	DetectionParams params;
	if(params.load("./config/detection-params.xml", "Basic sheet alignment algorithm") >= 0) {
		tlOss << "Successfully loaded detection params.";
		tlog.info(__FILE__, __LINE__, tlOss);

		std::ostringstream debugOss;
		debugOss << params;
		qDebug() << debugOss.str().c_str();

		for(const auto& iter : params.getParamTable()) {
			debugOss.str("");
			debugOss << "is " << iter.first << " a float? " << (params.isFloat(iter.first) ? "true." : "false.") << std::endl;
			debugOss << "is " << iter.first << " an int? " << (params.isInt(iter.first) ? "true." : "false.") << std::endl;
			qDebug() << debugOss.str().c_str();
		}

	} else {
		tlOss << "Failed to load detection params. See log.";
		tlog.fatal(__FILE__, __LINE__, tlOss);
	}

	SheetScan scan;
	if(scan.load("./test_images/filled.png") < 0) {
		tlOss << "Failed to load image.";
		tlog.fatal(__FILE__, __LINE__, tlOss);
	}

	scan.initDetection(params);
	scan.alignScan(params);
	scan.saveProcessedCache("./processed.png");
	scan.saveAnnotated("./annotated.png");





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
