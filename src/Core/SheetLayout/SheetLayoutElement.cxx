#include "SheetLayoutElement.hxx"

#include "TextLogging.hxx"
#include <sstream>

namespace {
	std::ostringstream tlOss;
	TextLogging tlog;
}

std::string SheetLayoutElement::toString() const {
	std::ostringstream oss;
	print(oss);
	return oss.str();
}

std::ostream& operator<<(std::ostream& os, const SheetLayoutElement& layoutElement) {
	layoutElement.print(os);
	return os;
}
