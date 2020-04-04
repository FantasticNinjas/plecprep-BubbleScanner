#include "SheetLayoutElement.hxx"

#include "TextLogging.hxx"
#include <sstream>

namespace {
	std::ostringstream tlOss;
	TextLogging tlog;
}

std::string EasyGrade::SheetLayoutElement::toString() const {
	std::ostringstream oss;
	print(oss);
	return oss.str();
}

std::ostream& operator<<(std::ostream& os, const EasyGrade::SheetLayoutElement& layoutElement) {
	layoutElement.print(os);
	return os;
}
