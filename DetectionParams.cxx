
#include <sstream>
#include "pugixml.hpp"

#include "TextLogging.hxx"
#include "DetectionParams.hxx"

namespace {
	std::ostringstream tlOss;
	TextLogging tlog;
}


DetectionParams::DetectionParams() = default;
DetectionParams::~DetectionParams() = default;

bool DetectionParams::hasParam(const std::string & paramName) const {
	auto iterator = paramTable_.find(paramName);
	return iterator != paramTable_.end();
}

bool DetectionParams::isFloat(const std::string& paramName) const {
	std::string strValue = getAsStr(paramName);
	return !strValue.empty() && strValue.find_first_not_of("0123456789.- ") == std::string::npos;
}
bool DetectionParams::isInt(const std::string& paramName) const {
	std::string strValue = getAsStr(paramName);
	return !strValue.empty() && strValue.find_first_not_of("0123456789- ") == std::string::npos;
}

float DetectionParams::getAsFloat(const std::string& paramName) const {
	//Note: The way this function is structered is a bit inefficient. getAsString gets called twice: once directly
	//and once by DetectionParams::isFloat. It's probably nothing to worry about, but it's worth being aware of.

	float floatValue{0.0};
	if(isFloat(paramName)) {
		floatValue = std::stof(getAsStr(paramName));
	} else {
		tlOss << "Failed to retrieve parameter \"" << paramName << "\" as a float.";
		tlog.warning(__FILE__, __LINE__, tlOss);
	}

	return floatValue;
}

int DetectionParams::getAsInt(const std::string& paramName) const {
	//Note: The way this function is structered is a bit inefficient. getAsString gets called twice: once directly
	//and once by DetectionParams::isFloat. It's probably nothing to worry about, but it's worth being aware of.

	int intValue{0};
	if(isInt(paramName)) {
		intValue = std::stoi(getAsStr(paramName));
	} else {
		tlOss << "Failed to retrieve parameter \"" << paramName << "\" as an int.";
		tlog.warning(__FILE__, __LINE__, tlOss);
	}

	return intValue;
}

std::string DetectionParams::getAsStr(const std::string& paramName) const {
	auto iterator = paramTable_.find(paramName);
	std::string value{};
	//Check that the parameter table contains an entry for paramName
	if(iterator == paramTable_.end()) {
		tlOss << "Attempted to retrieve nonexistant parameter \"" << paramName << "\" on filter config \"" << name_ << "\"";
		tlog.warning(__FILE__, __LINE__, tlOss);
	} else {
		value = iterator->second;
	}
	return value;
}

void DetectionParams::set(const std::string& paramName, float value) {
	set(paramName, std::to_string(value));
}

void DetectionParams::set(const std::string& paramName, int value) {
	set(paramName, std::to_string(value));
}

void DetectionParams::set(const std::string& paramName, const std::string& value) {
	tlOss << "Set parameter \"" << paramName << "\" to \"" << value << "\"";
	tlog.debug(__FILE__, __LINE__, tlOss);
	paramTable_[paramName] = value;
}


const std::map<std::string, std::string>& DetectionParams::getParamTable() const {
	return paramTable_;
}

int DetectionParams::load(const std::string& filename, const std::string& filterName) {

	int status = 0;

	//Parse XML file

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(filename.c_str());
	if(result) {
		tlOss << "Successfully parsed XML file \"" << filename << "\"";
		tlog.debug(__FILE__, __LINE__, tlOss);
	} else {
		status = -1;
		tlOss << "Failed to parse XML file \"" << filename << "\". PugiXML error message: " << result.description();
		tlog.critical(__FILE__, __LINE__, tlOss);
	}

	//Check that the XML file is formatted correctly.
	pugi::xml_node filterParamsNode;
	if(status >= 0) {
		filterParamsNode = doc.first_child();
		if(std::string(filterParamsNode.name()) != "filter-params") {
			status = -1;
			tlOss << "Failed to filter parameters. \"" << filename << "\" does not appear to be a filter configuration file. Root node type: \"" << filterParamsNode.name() << "\"";
			tlog.critical(__FILE__, __LINE__, tlOss);
		}
	}

	//Iterate through all of the filters in the configuration file to find the requested filter.
	pugi::xml_node filterNode;
	for(filterNode = filterParamsNode.child("filter"); filterParamsNode; filterNode = filterNode.next_sibling("filter")) {
		//Get name attribute of current filter
		pugi::xml_attribute nameAttr = filterNode.attribute("name");
		if(!nameAttr) {
			tlOss << "Encountered a filter with no name attribute while parsing filter configuration file \"" << filename << "\"";
			tlog.warning(__FILE__, __LINE__, tlOss);
			continue;
		}

		//Break out of the loop if the name of the current filter is the same as the one requesteed
		if(nameAttr.value() == filterName) {
			name_ = nameAttr.value();
			tlOss << "Found filter \"" << filterName << "\"";
			tlog.debug(__FILE__, __LINE__, tlOss);
			break;
		}
	}

	//Check that requested filter was in fact found. If not, then it is not present in the filter configuration file and the loop exited naturally having not found it.
	if(status >= 0 && !filterNode) {
		status = 1;
		tlOss << "Unable to find filter \"" << filterName << "\" in filter configuration file \"" << filename << "\"";
		tlog.critical(__FILE__, __LINE__, tlOss);
	}

	//Get the filter type
	if(status == 0) {
		pugi::xml_attribute typeAttr = filterNode.attribute("type");
		if(!typeAttr) {
			status = -1;
			tlOss << "Requested filter " << filterName << " does not have a type attribute \"" << filename << "\"";
			tlog.critical(__FILE__, __LINE__, tlOss);
		}
		//Note that it is okay to call typeAttr.value() even if typeAttr is null; it will just return an empty string
		filterType_ = parseFilterType(typeAttr.value());
	}

	//Load all of the parameters for the requested filter
	if(status == 0) {
		for(pugi::xml_node paramNode = filterNode.first_child(); paramNode; paramNode = paramNode.next_sibling()) {
			//Get the name of the current parameter
			std::string paramName = paramNode.name();
			if(paramTable_.find(paramName) != paramTable_.end()) {
				tlOss << "Overwriting existing parameter \"" << paramName << "\" on filter configuration \"" << filterName << "\"";
				tlog.warning(__FILE__, __LINE__, tlOss);
			}

			//Get the value of the current parameter
			std::string paramValue = paramNode.text().get();

			//Add the parameter to the list
			paramTable_[paramName] = paramValue;
		}
	}

	if(status == 0) {
		tlOss << "Successfully loaded parameter list for filter \"" << filterName << "\" from configuration file \"" << filename << "\"";
		tlog.info(__FILE__, __LINE__, tlOss);
	}
	return status;
}

void DetectionParams::reset() {
	name_ = "";
	filterType_ = FilterType::UNKNOWN;
	paramTable_.clear();
}

int DetectionParams::getFilterList(const std::string& filename, std::vector<std::string>& filters) {
	return 0;
}

const std::string& DetectionParams::getName() const {
	return name_;
}

FilterType DetectionParams::getFilterType() const {
	return filterType_;
}

std::string toString(const FilterType& filterType) {
	std::ostringstream oss;
	oss << filterType;
	return oss.str();
}

FilterType parseFilterType(const std::string& str) {
	if(str == toString(FilterType::THRESH_FRAC)) {
		return FilterType::THRESH_FRAC;
	} else if(str == toString(FilterType::THRESH_CONTOUR)) {
		return FilterType::THRESH_CONTOUR;
	} else if(str == toString(FilterType::THRESH_HCIRCLES)) {
		return FilterType::THRESH_HCIRCLES;
	} else if(str == toString(FilterType::UNKNOWN)) {
		return FilterType::UNKNOWN;
	} else {
		tlOss << "Encountered unhandled filter type.";
		tlog.warning(__FILE__, __LINE__, tlOss);
		return FilterType::UNKNOWN;
	}
}

std::ostream& operator<<(std::ostream& os, const FilterType& filterType) {
	switch(filterType) {
	case FilterType::THRESH_FRAC:
		os << "THRESH_FRAC";
		break;
	case FilterType::THRESH_CONTOUR:
		os << "THRESH_CONTOUR";
		break;
	case FilterType::UNKNOWN:
		os << "UNKNOWN";
		break;
	case FilterType::THRESH_HCIRCLES:
		os << "THRESH_HCIRCLES";
		break;
	default:
		os << "UNKNOWN";
		tlOss << "Encountered unhandled filter type.";
		tlog.warning(__FILE__, __LINE__, tlOss);
	}
	return os;
}

std::ostream& operator<<(std::ostream& os, const DetectionParams& detectionParams) {
	os << "Detection Algorithm Configuration \"" << detectionParams.getName() << "\" (" << detectionParams.getFilterType() << ")" << std::endl;
	for(const auto& iter : detectionParams.getParamTable()) {
		os << "    " << iter.first << ": " << iter.second << std::endl;
	}
	return os;
}


