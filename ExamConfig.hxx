#pragma once

#include "DetectionParams.hxx"
#include "SheetLayout.hxx"

class ExamConfig {
public:
	ExamConfig();
	~ExamConfig();

	int setAlignmentAlgorithm(const std::string& algorithmName);
	int setDetectionAlgorithm(const std::string& algorithmName);
private:

	DetectionParams alignmentAlgorithm_;
	DetectionParams detectionAlgorithm_;
	SheetLayout sheetLayout_;

};

