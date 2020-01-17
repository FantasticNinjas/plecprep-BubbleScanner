#pragma once

#include "TextLogging.hxx"

class QWidget;

class QtLogging {
public:
	QtLogging();
	~QtLogging();

	void debug(char *file, int line, QWidget* parent, std::ostringstream& tlOss);
	void info(char *file, int line, QWidget* parent, std::ostringstream& tlOss);
	void warning(char *file, int line, QWidget* parent, std::ostringstream& tlOss);
	void critical(char *file, int line, QWidget* parent, std::ostringstream& tlOss);
	void fatal(char *file, int line, QWidget* parent, std::ostringstream& tlOss);

	void setAreDebugDialogsEnabled(bool isEnabled);
	void setAreInfoDialogsEnabled(bool isEnabled);
	void setAreWarningDialogsEnabled(bool isEnabled);
	void setAreCriticalDialogsEnabled(bool isEnabled);
	void setAreFatalDialogsEnabled(bool isEnabled);

	static void setAreDebugDialogsEnabledDefault(bool isEnabled);
	static void setAreInfoDialogsEnabledDefault(bool isEnabled);
	static void setAreWarningDialogsEnabledDefault(bool isEnabled);
	static void setAreCriticalDialogsEnabledDefault(bool isEnabled);
	static void setAreFatalDialogsEnabledDefault(bool isEnabled);

private:
	TextLogging tlog;

	bool areDebugDialogsEnabled_{false};
	bool areInfoDialogsEnabled_{false};
	bool areWarningDialogsEnabled_{false};
	bool areCriticalDialogsEnabled_{false};
	bool areFatalDialogsEnabled_{false};

	static bool areDebugDialogsEnabledDefault_;
	static bool areInfoDialogsEnabledDefault_;
	static bool areWarningDialogsEnabledDefault_;
	static bool areCriticalDialogsEnabledDefault_;
	static bool areFatalDialogsEnabledDefault_;
	
};

