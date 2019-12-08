#pragma once

#include <string>
#include <fstream>

class TextLogging {
public:
	TextLogging();
	~TextLogging();

	void debug(char *file, int line, std::ostringstream& tlOss);
	void info(char *file, int line, std::ostringstream& tlOss);
	void warning(char *file, int line, std::ostringstream& tlOss);
	void critical(char *file, int line, std::ostringstream& tlOss);
	void fatal(char *file, int line, std::ostringstream& tlOss);

	///
	/// @brief: Set whether DEBUG entries should be included in the log
	///
	void setIsDebugVerbosityEnabled(bool isEnabled);

	///
	/// @brief: Set whether INFO statements should be included in the log
	///
	void setIsInfoVerbosityEnabled(bool isEnabled);

	///
	/// @brief: Set whether WARN statements should be included in the log
	///
	void setIsWarningVerbosityEnabled(bool isEnabled);

	///
	/// @brief: Set whether CRIT statements should be included in the log
	///
	void setIsCriticalVerbosityEnabled(bool isEnabled);

	///
	/// @brief: Set whether FATAL statements should be included in the log. Note: fatal errors will kill the program either way.
	///
	void setIsFatalVerbosityEnabled(bool isEnabled);

	///
	/// @brief: Set whether log entries should be color coded using ANSI color codes.
	///
	/// @note: to view log file in color, use 'echo -ne $(cat log.txt | sed  's/$/\\n/')'
	///
	void setIsColorTextEnabled(bool isEnabled);


	///
	/// @brief: Set whether DEBUG entries should be enabled by default for new instances of TextLogging (for the rest of this session)
	///
	static void setIsDebugVerbosityEnabledDefault(bool isEnabled);

	///
	/// @brief: Set whether INFO entries should be enabled by default for new instances of TextLogging (for the rest of this session)
	///
	static void setIsInfoVerbosityEnabledDefault(bool isEnabled);

	///
	/// @brief: Set whether WARN entries should be enabled by default for new instances of TextLogging (for the rest of this session)
	///
	static void setIsWarningVerbosityEnabledDefault(bool isEnabled);

	///
	/// @brief: Set whether CRIT entries should be enabled by default for new instances of TextLogging (for the rest of this session)
	///
	static void setIsCriticalVerbosityEnabledDefault(bool isEnabled);

	///
	/// @brief: Set whether FATAL entries should be enabled by default for new instances of TextLogging (for the rest of this session)
	///
	static void setIsFatalVerbosityEnabledDefault(bool isEnabled);

	///
	/// @brief: Set whether ANSI colors should be enabled by default for new instances of TextLogging (for the rest of this session)
	///
	static void setIsColorTextEnabledDefault(bool isEnabled);

	static std::string getLogFileName();

private:

	enum class LogLevel {
		DEBUG,
		INFO,
		WARNING,
		CRITICAL,
		FATAL
	};

	void log(char *file, int line, std::ostringstream& tlOss, LogLevel level);
	static std::string generateLogFileName();

	static std::string logDir_;
	static std::string logName_;
	static std::ofstream logFile_;

	bool isDebugVerbosityEnabled_;
	bool isInfoVerbosityEnabled_;
	bool isWarningVerbosityEnabled_;
	bool isCriticalVerbosityEnabled_;
	bool isFatalVerbosityEnabled_;
	bool isColorTextEnabled_;

	static bool isDebugVerbosityEnabledDefault_;
	static bool isInfoVerbosityEnabledDefault_;
	static bool isWarningVerbosityEnabledDefault_;
	static bool isCriticalVerbosityEnabledDefault_;
	static bool isFatalVerbosityEnabledDefault_;
	static bool isColorTextEnabledDefault_;
};

