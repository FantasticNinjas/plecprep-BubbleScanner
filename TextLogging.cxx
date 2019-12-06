
#include <sstream>
#include <csignal>
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>

#include "TextLogging.hxx"

const std::string DEFAULT_COLOR = "\033[0m";
const std::string WHITE_COLOR = "\033[1;37m";
const std::string RED_COLOR = "\033[1;31m";
const std::string GREEN_COLOR = "\033[0;32m";
const std::string YELLOW_COLOR = "\033[1;33m";
const std::string MAGENTA_COLOR = "\033[1;35m";

bool TextLogging::isDebugVerbosityEnabledDefault_ = false;
bool TextLogging::isInfoVerbosityEnabledDefault_ = true;
bool TextLogging::isWarningVerbosityEnabledDefault_ = true;
bool TextLogging::isCriticalVerbosityEnabledDefault_ = true;
bool TextLogging::isFatalVerbosityEnabledDefault_ = true;
bool TextLogging::isColorTextEnabledDefault_ = true;

std::string TextLogging::logDir_ = "./logs/";
std::string TextLogging::logName_ = generateLogFileName();


TextLogging::TextLogging() {
	isDebugVerbosityEnabled_ = isDebugVerbosityEnabledDefault_;
	isInfoVerbosityEnabled_ = isInfoVerbosityEnabledDefault_;
	isWarningVerbosityEnabled_ = isWarningVerbosityEnabledDefault_;
	isCriticalVerbosityEnabled_ = isCriticalVerbosityEnabledDefault_;
	isFatalVerbosityEnabled_ = isFatalVerbosityEnabledDefault_;
	isColorTextEnabled_ = isColorTextEnabledDefault_;
}


TextLogging::~TextLogging() = default;

void TextLogging::debug(char * file, int line, std::ostringstream& tlOss) {
	if(isDebugVerbosityEnabled_ || isDebugVerbosityEnabledDefault_) {
		log(file, line, tlOss, LogLevel::DEBUG);
	}
	//Reset the supplied ostringstream so that it can be reused.
	tlOss.str("");
}

void TextLogging::info(char * file, int line, std::ostringstream& tlOss) {
	if(isInfoVerbosityEnabled_ || isInfoVerbosityEnabledDefault_) {
		log(file, line, tlOss, LogLevel::INFO);
	}
	//Reset the supplied ostringstream so that it can be reused.
	tlOss.str("");
}

void TextLogging::warning(char * file, int line, std::ostringstream& tlOss) {
	if(isWarningVerbosityEnabled_ || isWarningVerbosityEnabledDefault_) {
		log(file, line, tlOss, LogLevel::WARNING);
	}
	//Reset the supplied ostringstream so that it can be reused.
	tlOss.str("");
}

void TextLogging::critical(char * file, int line, std::ostringstream& tlOss) {
	if(isCriticalVerbosityEnabled_ || isCriticalVerbosityEnabledDefault_) {
		log(file, line, tlOss, LogLevel::CRITICAL);
	}
	//Reset the supplied ostringstream so that it can be reused.
	tlOss.str("");
}

void TextLogging::fatal(char * file, int line, std::ostringstream& tlOss) {
	if(isFatalVerbosityEnabled_ || isFatalVerbosityEnabledDefault_) {
		log(file, line, tlOss, LogLevel::FATAL);
	}
	//Reset the supplied ostringstream so that it can be reused.
	tlOss.str("");
	//Kill the program if a fatal error occurs
	raise(SIGTERM);
}

void TextLogging::log(char * file, int line, std::ostringstream & tlOss, LogLevel level) {

	//Open log file
	std::ofstream logFile{};
	logFile.open(getLogFileName(), std::ios_base::app);

	//Set the text to the appropriate color (if color text is enabled)
	if(isColorTextEnabled_) {
		switch(level) {
		case LogLevel::DEBUG:
			logFile << GREEN_COLOR;
			break;
		case LogLevel::INFO:
			logFile << WHITE_COLOR;
			break;
		case LogLevel::WARNING:
			logFile << YELLOW_COLOR;
			break;
		case LogLevel::CRITICAL:
			logFile << MAGENTA_COLOR;
			break;
		case LogLevel::FATAL:
			logFile << RED_COLOR;
			break;
		default:
			logFile << DEFAULT_COLOR;
		}
	}

	//Log the warning severity
	switch(level) {
	case LogLevel::DEBUG:
		logFile << "DEBUG: ";
		break;
	case LogLevel::INFO:
		logFile << "INFO: ";
		break;
	case LogLevel::WARNING:
		logFile << "WARN: ";
		break;
	case LogLevel::CRITICAL:
		logFile << "CRIT: ";
		break;
	case LogLevel::FATAL:
		logFile << "FATAL: ";
		break;
	default:
		logFile << ": ";
	}

	//Log the file/line info
	logFile << file << " #" << line << ": ";

	//Log the warning message
	logFile << tlOss.str();

	//Reset the text color
	if(isColorTextEnabled_) {
		logFile << DEFAULT_COLOR;
	}

	logFile << std::endl;

	//Write the new log entry to the log file.
	logFile.close();
}

std::string TextLogging::generateLogFileName() {
	std::ostringstream logName;
	logName << "ScannerLog-";

	std::time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	logName << std::put_time(std::localtime(&currentTime), "%Y-%m-%d-%H-%M-%S");;

	logName << ".txt";

	return logName.str();
}

///////////////////////////////////////
//        Getters and Setters        //
///////////////////////////////////////

std::string TextLogging::getLogFileName() {
	return logDir_ + logName_;
}

void TextLogging::setIsDebugVerbosityEnabled(bool isEnabled) {
	isDebugVerbosityEnabled_ = isEnabled;
}

void TextLogging::setIsInfoVerbosityEnabled(bool isEnabled) {
	isInfoVerbosityEnabled_ = isEnabled;
}

void TextLogging::setIsWarningVerbosityEnabled(bool isEnabled) {
	isWarningVerbosityEnabled_ = isEnabled;
}

void TextLogging::setIsCriticalVerbosityEnabled(bool isEnabled) {
	isCriticalVerbosityEnabled_ = isEnabled;
}

void TextLogging::setIsFatalVerbosityEnabled(bool isEnabled) {
	isFatalVerbosityEnabled_ = isEnabled;
}

void TextLogging::setIsColorTextEnabled(bool isEnabled) {
	isColorTextEnabled_ = isEnabled;
}

// Setters for defaults

void TextLogging::setIsDebugVerbosityEnabledDefault(bool isEnabled) {
	isDebugVerbosityEnabledDefault_ = isEnabled;
}

void TextLogging::setIsInfoVerbosityEnabledDefault(bool isEnabled) {
	isInfoVerbosityEnabledDefault_ = isEnabled;
}

void TextLogging::setIsWarningVerbosityEnabledDefault(bool isEnabled) {
	isWarningVerbosityEnabledDefault_ = isEnabled;
}

void TextLogging::setIsCriticalVerbosityEnabledDefault(bool isEnabled) {
	isCriticalVerbosityEnabledDefault_ = isEnabled;
}

void TextLogging::setIsFatalVerbosityEnabledDefault(bool isEnabled) {
	isFatalVerbosityEnabledDefault_ = isEnabled;
}

void TextLogging::setIsColorTextEnabledDefault(bool isEnabled) {
	isColorTextEnabledDefault_ = isEnabled;
}
