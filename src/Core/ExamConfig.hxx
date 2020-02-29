#pragma once

#include "DetectionParams.hxx"
#include "ScanSheetLayout.hxx"

class ExamConfig {
public:
	ExamConfig();
	~ExamConfig();

	int setAlignmentAlgorithm(const std::string& algorithmName);
	int setDetectionAlgorithm(const std::string& algorithmName);
private:

	DetectionParams alignmentAlgorithm_;
	DetectionParams detectionAlgorithm_;
	ScanSheetLayout sheetLayout_;

};

