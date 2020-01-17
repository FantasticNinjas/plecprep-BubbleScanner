#include "ExamConfig.hxx"
#include "FilenameOracle.hxx"



ExamConfig::ExamConfig() = default;
ExamConfig::~ExamConfig() = default;

int ExamConfig::setAlignmentAlgorithm(const std::string& algorithmName) {
	alignmentAlgorithm_.reset();
	return alignmentAlgorithm_.load(FilenameOracle::getAlignmentAlgorithmsFilename(), algorithmName);
}

int ExamConfig::setDetectionAlgorithm(const std::string& algorithmName) {
	detectionAlgorithm_.reset();
	return detectionAlgorithm_.load(FilenameOracle::getDetectionAlgorithmsFilename(), algorithmName);
}
