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

	void reset();

	///
	/// <summary> Get a list of the side layouts on this sheet layout </summary>
	///
	const std::vector<struct SideLayout>& getSideLayouts() const;
	const std::string& getTitle() const;
	const std::string& getBackgroundImageFilename() const;

	///
	/// <summary> Get the title of a sheet layout from its file path. This function may be desirable because it only parses a small portion of the sheet layout and will therefore be more efficient. </summary>
	///
	/// <param name="filename"> The filename of the sheet layout file </param>
	///
	/// <return> The title of the sheet layout, or an empty string if an error occured. </return>
	///
	static std::string getLayoutTitle(const std::string& filename);

private:
	std::string title_;
	std::string backgroundImage_;
	std::vector<struct SideLayout> sideLayouts_;
};

struct SideLayout {

	inline void addQuestionGroup(const struct QuestionGroupLayout& questionGroup) {
		questionGroups_.push_back(questionGroup);
	}

	inline size_t getNumGroups() {
		return questionGroups_.size();
	}

	size_t sideNumber_;
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

	cv::Rect2f boundingBox() const;

	inline void addQuestion(const struct QuestionLayout& question) {
		questions_.push_back(question);
	}

	inline size_t getNumQuestions() {
		return questions_.size();
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

	cv::Rect2f boundingBox() const;

	inline void addBubble(const struct BubbleLayout& bubble) {
		bubbles_.push_back(bubble);
	}

	std::vector<struct BubbleLayout> bubbles_;
};

///
/// @brief Represents a single bubble on a sheet. Should always be part of a QuestionLayout.
///
struct BubbleLayout {
	cv::Rect2f boundingBox() const;

	std::string answer_;
	cv::Vec3f location_;
};

std::ostream& operator<<(std::ostream& os, const SheetLayout& sheetLayout);

