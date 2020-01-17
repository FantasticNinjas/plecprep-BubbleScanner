#pragma once

#include <map>
#include <string>
#include <vector>

enum class FilterType {
	UNKNOWN,
	THRESH_FRAC,
	THRESH_CONTOUR,
	THRESH_HCIRCLES
};

std::string toString(const FilterType& filterType);
FilterType parseFilterType(const std::string& str);
std::ostream& operator<<(std::ostream& os, const FilterType& filterType);

class DetectionParams {
public:
	DetectionParams();
	~DetectionParams();


	///
	/// <summary> Check whether a parameter is present on this configuration </summary>
	///
	/// <param name="paramName"> The name of the parameter to search for </param>
	///
	/// <returns> True if the requested parameter exists, else false. </returns>
	///
	bool hasParam(const std::string& paramName) const;

	///
	/// <summary> Check whether the value of a parameter can be interpreted as a float. </summary>
	///
	/// <param name="paramName"> The name of the parameter to search for </param>
	///
	/// <returns> True if the requested parameter exists and can be successfully converted to a float, else false. </returns>
	///
	bool isFloat(const std::string& paramName) const;

	///
	/// <summary> Check whether the value of a parameter can be interpreted as an int. </summary>
	///
	/// <param name="paramName"> The name of the parameter to search for </param>
	///
	/// <returns> True if the requested parameter exists and can be successfully converted to an int, else false. </returns>
	///
	bool isInt(const std::string& paramName) const;

	///
	/// <summary> Get the value of a parameter as a float. Will log a warning if the parameter does not exist in this instance or if the
	///           parameter value cannot be parsed as a float </summary>
	///
	/// <param name="paramName"> The name of the parameter to search for </param>
	///
	/// <returns> A The value of the requested parameter represented as a float. Zero if an error occured. </returns>
	///
	/// <note> There is no way for this function to report an error to the rest of the program. As such, it should only be called when
	///        you can already be sure that it will succeed (i.e. the parameter you are requesting is present and can be converted to a
	///        float). This can be checked with the DetectionParams::isFloat() method. </note>
	///
	float getAsFloat(const std::string& paramName) const;


	///
	/// <summary> Get the value of a parameter as an int. Will log a warning if the parameter does not exist in this instance or if the
	///           parameter value cannot be parsed as an int </summary>
	///
	/// <param name="paramName"> The name of the parameter to search for </param>
	///
	/// <returns> A The value of the requested parameter represented as an int. Zero if an error occured. </returns>
	///
	/// <note> There is no way for this function to report an error to the rest of the program. As such, it should only be called when
	///        you can already be sure that it will succeed (i.e. the parameter you are requesting is present and can be converted to a
	///        int). This can be checked with the DetectionParams::isInt() method. </note>
	///
	int getAsInt(const std::string& paramName) const;

	///
	/// @brief Get the value of a parameter as a string. Will log an error if the requested parameter does not exist.
	///
	/// @param[in] paramName The name of the parameter to search for
	///
	/// @return A The value of the requested parameter represented as a string. An empty string if an error occured
	///
	std::string getAsStr(const std::string& paramName) const;

	void set(const std::string& paramName, float value);
	void set(const std::string& paramName, int value);
	void set(const std::string& paramName, const std::string& value);

	///
	/// @brief Get a reference to a map of every parameter and its value
	///
	const std::map<std::string, std::string>& getParamTable() const;

	///
	/// <summary> Extract parameters for a specific detection algorithm configuration from a filter config file </summary>
	///
	/// <param name="filename"> filename The name of the filter configuration file </param>
	/// <param name="filterName"> filterName The name of the configuration for which to extract parameter values </param>
	///
	/// <returns> integer status code. 0 if the detection algorithm configuration was read successfully, positive if the
	///         requested configuration could not be found, negative if an error occured. </returns>
	///
	int load(const std::string& filename, const std::string& filterName);

	///
	/// <summary> Remove all of the parameters and reset the object </summary>
	///
	void reset();

	///
	/// @brief Get a list of all detection algorithm configurations present in a particular filter config file
	///
	/// @param[in] filename The name of the filter config file
	/// @param[out] filters Vector in which the list of detection algorithm configuration names will be placed
	///
	/// @return Integer status code. Negative if an error occured, non-negative if no error occured.
	static int getFilterList(const std::string& filename, std::vector<std::string>& filters);

	const std::string& getName() const;
	FilterType getFilterType() const;

private:
	std::string name_{};
	FilterType filterType_{};
	std::map<std::string, std::string> paramTable_{};
};

std::ostream& operator<<(std::ostream& os, const DetectionParams& detectionParams);

