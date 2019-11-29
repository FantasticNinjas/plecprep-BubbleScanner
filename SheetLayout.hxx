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


///
/// @brief Represents a single bubble on a sheet. Should always be part of a QuestionLayout.
///
struct BubbleLayout {
	cv::Rect2f boundingBox() const;

	std::string answer_;
	cv::Vec3f location_;
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
		questions_.back().questionNumber_ = questions_.size() - 1;
	}

	inline void insertQuestion(const struct QuestionLayout& question, int position) {
		//Get an iterator representing the position at which the question should be inserted
		auto it = questions_.begin() + position;
		if(it >= questions_.end()) {
			//If the requested position is out of range, just put the question at the end.
			addQuestion(question);
		} else {
			//If the requestion position is valid, insert the question
			questions_.insert(it, question);
			
			//Update the question number for the newly added question and all others that follow it
			for(int i = position; i < questions_.size(); i++) {
				questions_[i].questionNumber_ = i;
			}
		}
	}

	inline void removeQuestion(const struct QuestionLayout& question) {
		//Get the question number of the question to be removed (which should also be its index in the questions list)
		int questionNumber = question.questionNumber_;
		if(questionNumber < questions_.size()) {
			//If question number is valid, remove the question at that index
			questions_.erase(questions_.begin() + questionNumber);
			//Update the question numbers of all subsequent questions
			for(int i = questionNumber; i < questions_.size(); i++) {
				questions_[i].questionNumber_ = i;
			}
		}
	}

	inline size_t getNumQuestions() {
		return questions_.size();
	}

	std::string name_;
	ContentType contentType_;
	std::vector<struct QuestionLayout> questions_;
};

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
	std::vector<struct SideLayout>& getSideLayouts();
	int numSideLayouts() const;
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

std::ostream& operator<<(std::ostream& os, const SheetLayout& sheetLayout);

