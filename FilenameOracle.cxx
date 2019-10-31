#include "FilenameOracle.hxx"


std::string FilenameOracle::layoutDirectoryFilename_ = "./config/layouts/";
std::string FilenameOracle::detectionAlgorithmsFilename_ = "./config/detection-algorithms.xml";
std::string FilenameOracle::alignmentAlgorithmsFilename_ = "./config/alignment-algorithms.xml";
std::string FilenameOracle::circleAlgorithmsFilename_ = "./config/circle-algorithms.xml";

const std::string& FilenameOracle::getLayoutDirectoryFilename() {
	return layoutDirectoryFilename_;
}

const std::string& FilenameOracle::getDetectionAlgorithmsFilename() {
	return detectionAlgorithmsFilename_;
}

const std::string& FilenameOracle::getAlignmentAlgorithmsFilename() {
	return alignmentAlgorithmsFilename_;
}

const std::string& FilenameOracle::getCircleAlgorithmsFilename() {
	return circleAlgorithmsFilename_;
}