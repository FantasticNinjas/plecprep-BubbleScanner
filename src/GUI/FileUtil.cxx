
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
