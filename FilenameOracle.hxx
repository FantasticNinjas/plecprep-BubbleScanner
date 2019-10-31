#pragma once

#include <string>

class FilenameOracle {
public:
	static const std::string& getLayoutDirectoryFilename();
	static const std::string& getDetectionAlgorithmsFilename();
	static const std::string& getAlignmentAlgorithmsFilename();
	static const std::string& getCircleAlgorithmsFilename();
private:
	static std::string layoutDirectoryFilename_;
	static std::string detectionAlgorithmsFilename_;
	static std::string alignmentAlgorithmsFilename_;
	static std::string circleAlgorithmsFilename_;
};

