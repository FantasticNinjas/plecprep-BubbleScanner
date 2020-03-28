
#include <QFile>
#include <sstream>

#include "FileUtil.hxx"
#include "TextLogging.hxx"

namespace {
	std::ostringstream tlOss;
	TextLogging tlog;
}


std::ifstream FileUtil::getInputStream(QFile& file) {
	if(file.isReadable()) {
		return std::ifstream(::_fdopen(file.handle(), "r"));
	} else {
		tlOss << "Unable to open input stream to file as it is not readable.";
		tlog.critical(__FILE__, __LINE__, tlOss);
		return std::ifstream();
	}
}

std::ofstream FileUtil::getOutputStream(QFile & file) {
	//This implementation does not work for unknown reasons
	//The file always gets truncated to 4kB

	//TODO: figure out why this doesn't work and fix it

	if(file.isWritable()) {
		return std::ofstream(::_fdopen(file.handle(), "a+"));
	} else {
		tlOss << "Unable to open output stream to file as it is not writable.";
		tlog.critical(__FILE__, __LINE__, tlOss);
		return std::ofstream();
	}
}
