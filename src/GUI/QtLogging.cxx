#include <QMessageBox>
#include <sstream>

#include "QtLogging.hxx"

//Set default values for static 'default' variables
bool QtLogging::areDebugDialogsEnabledDefault_ = false;
bool QtLogging::areInfoDialogsEnabledDefault_ = true;
bool QtLogging::areWarningDialogsEnabledDefault_ = true;
bool QtLogging::areCriticalDialogsEnabledDefault_ = true;
bool QtLogging::areFatalDialogsEnabledDefault_ = true;


QtLogging::QtLogging() = default;
QtLogging::~QtLogging() = default;

void QtLogging::debug(char * file, int line, QWidget* parent, std::ostringstream & tlOss) {
	if(areDebugDialogsEnabled_ || areDebugDialogsEnabledDefault_) {
		QMessageBox::information(parent, "Debugging Info", QString::fromStdString("Debugging Message: " + tlOss.str()));
	}
	tlog.debug(file, line, tlOss);
}

void QtLogging::info(char * file, int line, QWidget* parent, std::ostringstream & tlOss) {
	if(areInfoDialogsEnabled_ || areInfoDialogsEnabledDefault_) {
		QMessageBox::information(parent, "Bubble Scanner", QString::fromStdString(tlOss.str()));
	}
	tlog.info(file, line, tlOss);
}

void QtLogging::warning(char * file, int line, QWidget* parent, std::ostringstream & tlOss) {
	if(areWarningDialogsEnabled_ || areWarningDialogsEnabledDefault_) {
		QMessageBox::warning(parent, "Warning", QString::fromStdString(tlOss.str()));
	}
	tlog.warning(file, line, tlOss);
}

void QtLogging::critical(char * file, int line, QWidget* parent, std::ostringstream & tlOss) {
	if(areCriticalDialogsEnabled_ || areCriticalDialogsEnabledDefault_) {
		QMessageBox::critical(parent, "Error", QString::fromStdString(tlOss.str()));
	}
	tlog.critical(file, line, tlOss);
}

void QtLogging::fatal(char * file, int line, QWidget* parent, std::ostringstream & tlOss) {
	if(areFatalDialogsEnabled_ || areFatalDialogsEnabledDefault_) {
		QMessageBox::critical(parent, "Fatal Error", QString::fromStdString(tlOss.str()));
	}
	tlog.critical(file, line, tlOss);
}

void QtLogging::setAreDebugDialogsEnabled(bool isEnabled) {
	areDebugDialogsEnabled_ = isEnabled;
}

void QtLogging::setAreInfoDialogsEnabled(bool isEnabled) {
	areInfoDialogsEnabled_ = isEnabled;
}

void QtLogging::setAreWarningDialogsEnabled(bool isEnabled) {
	areWarningDialogsEnabled_ = isEnabled;
}

void QtLogging::setAreCriticalDialogsEnabled(bool isEnabled) {
	areCriticalDialogsEnabled_ = isEnabled;
}

void QtLogging::setAreFatalDialogsEnabled(bool isEnabled) {
	areFatalDialogsEnabled_ = isEnabled;
}

void QtLogging::setAreDebugDialogsEnabledDefault(bool isEnabled) {
	areDebugDialogsEnabledDefault_ = isEnabled;
}

void QtLogging::setAreInfoDialogsEnabledDefault(bool isEnabled) {
	areInfoDialogsEnabledDefault_ = isEnabled;
}

void QtLogging::setAreWarningDialogsEnabledDefault(bool isEnabled) {
	areWarningDialogsEnabledDefault_ = isEnabled;
}

void QtLogging::setAreCriticalDialogsEnabledDefault(bool isEnabled) {
	areCriticalDialogsEnabledDefault_ = isEnabled;
}

void QtLogging::setAreFatalDialogsEnabledDefault(bool isEnabled) {
	areFatalDialogsEnabledDefault_ = isEnabled;
}
