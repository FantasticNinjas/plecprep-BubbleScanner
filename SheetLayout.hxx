// 
// University of Massachusetts Amherst Physics Lecture Prep
//
// Project: ScantronReader
// File: SheetLayout.hxx
// Author: Casey Massar (caseymassar@gmail.com)
// Description: Stores the layout of a scanner sheet (i.e. the size and location of anwer bubbles and how they are grouped into questions)
//

#pragma once

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>


class SheetLayout {
public:
	SheetLayout();
	~SheetLayout();

	///
	/// @brief Load sheet layout from an XML file
	///
	/// @param[in] filename The name of the XML file to load
	///
	/// @return Integer status code. Negative indicates that an error occured, non-negative indicates that no error occured.
	///
	int load(const std::string& filename);

	///
	/// @brief Save sheet layout to an XML file
	///
	/// @param[in] filename Where to save the XML file
	///
	/// @return Integer status code. Negative indicates that an error occured, non-negative indicates that no error occured.
	///
	int save(const std::string& filename) const;

	const std::vector<struct QuestionGroupLayout>& getQuestionGroups() const;
	const std::string& getTitle() const;

private:
	std::string title_;
	std::vector<struct QuestionGroupLayout> questionGroups_;
};

///
/// @brief Represents a list of related questions on a sheet. For instance, "all of the questions (one for each letter) that make up the person's name," could
///        be a QuestionGroupLayout, as could "all of the actual numbered questions." 
///
struct QuestionGroupLayout {
	enum class ContentType {
		UNKNOWN,
		PROBLEMS,
		STRING
	};

	inline void addQuestion(const struct QuestionLayout& question) {
		questions_.push_back(question);
	}

	std::string name_;
	ContentType contentType_;
	std::vector<struct QuestionLayout> questions_;
};

///
/// @brief Represents a single question on a sheet. Should always be part of a QuestionGroupLayout
///
struct QuestionLayout {
	size_t questionNumber_;

	inline void addBubble(const struct BubbleLayout& bubble) {
		bubbles_.push_back(bubble);
	}

	std::vector<struct BubbleLayout> bubbles_;
};

///
/// @brief Represents a single bubble on a sheet. Should always be part of a QuestionLayout.
///
struct BubbleLayout {
	std::string answer_;
	cv::Vec3f location_;
};

std::ostream& operator<<(std::ostream& os, const SheetLayout& sheetLayout);

