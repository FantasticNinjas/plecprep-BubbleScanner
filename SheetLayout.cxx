
#include <sstream>

#include "pugixml.hpp"

#include "SheetLayout.hxx"
#include "TextLogging.hxx"

namespace {
	static std::ostringstream tlOss;
	static TextLogging tlog;
}

SheetLayout::SheetLayout() = default;


SheetLayout::~SheetLayout() = default;

int SheetLayout::load(const std::string & filename) {

	//std::ostringstream tlOss;
	//TextLogging tlog;

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

	//Load sheet title

	pugi::xml_node sheetNode;
	if(status >= 0) {
		sheetNode = doc.first_child();
		if(std::string(sheetNode.name()) != "sheet") {
			status = -1;
			tlOss << "Failed to load sheet layout. \"" << filename << "\" does not appear to be a template. Root node type: \"" << sheetNode.name() << "\"";
			tlog.critical(__FILE__, __LINE__, tlOss);
		}
	}

	pugi::xml_attribute sheetTitleAttr;
	if(status >= 0) {
		sheetTitleAttr = sheetNode.attribute("title");
		if(sheetTitleAttr) {
			title_ = sheetTitleAttr.value();
		} else {
			title_ = "";
			tlOss << "Sheet layout loaded from \"" << filename << "\" does not have a name.";
			tlog.warning(__FILE__, __LINE__, tlOss);
		}
	}

	//Load question groups

	pugi::xml_node questionGroupsNode;
	if(status >= 0) {
		questionGroupsNode = sheetNode.child("question-groups");
		if(!questionGroupsNode || !questionGroupsNode.child("group")) {
			status = 1;
			tlOss << "Sheet layout \"" << title_ << "\" loaded from \"" << filename << "\" does not contain any question groups";
			tlog.warning(__FILE__, __LINE__, tlOss);
		}
	}

	//Iterate over all question groups
	if(status == 0) {
		for(pugi::xml_node groupNode = questionGroupsNode.child("group"); groupNode; groupNode = groupNode.next_sibling("group")) {

			//Create a new QuestionGroupLayout to be populated and then added to the SheetLayout.
			struct QuestionGroupLayout currentQuestionGroup;

			//Set question group name
			pugi::xml_attribute groupNameAttr = groupNode.attribute("name");
			if(groupNameAttr) {
				currentQuestionGroup.name_ = groupNameAttr.value();
			} else {
				status = -1;
				tlOss << "Encountered a question group without a name attribute while parsing sheet layout \"" << title_ << "\" from file \"" << filename << "\"";
				tlog.critical(__FILE__, __LINE__, tlOss);
			}

			//Set question group content type
			pugi::xml_attribute groupTypeAttr = groupNode.attribute("type");
			if(groupTypeAttr) {
				if(groupTypeAttr.value() == std::string("string")) {
					currentQuestionGroup.contentType_ = QuestionGroupLayout::ContentType::STRING;
				} else if(groupTypeAttr.value() == std::string("problems")) {
					currentQuestionGroup.contentType_ = QuestionGroupLayout::ContentType::PROBLEMS;
				} else {
					currentQuestionGroup.contentType_ = QuestionGroupLayout::ContentType::UNKNOWN;

					status = -1;
					tlOss << "Question group \"" << currentQuestionGroup.name_ << "\" has an unrecognized content type \"" << groupTypeAttr.value() << "\"";
					tlog.critical(__FILE__, __LINE__, tlOss);
				}
			} else {
				status = -1;
				tlOss << "Question group \"" << currentQuestionGroup.name_ << "\" does not specify a content type.";
				tlog.critical(__FILE__, __LINE__, tlOss);
			}

			//Add questions to group

			if(!groupNode.child("question")) {
				tlOss << "Question group \"" << currentQuestionGroup.name_ << "\" does not contain any questions.";
				tlog.warning(__FILE__, __LINE__, tlOss);
			}

			size_t i = 0;
			for(pugi::xml_node questionNode = groupNode.child("question"); questionNode; questionNode = questionNode.next_sibling("question")) {
				struct QuestionLayout currentQuestion;

				currentQuestion.questionNumber_ = i;

				//Add bubbles to question

				if(!questionNode.child("bubble")) {
					status = -1;
					tlOss << "Question " << currentQuestion.questionNumber_ << " in \"" << currentQuestionGroup.name_ << "\" does not list any bubbles.";
					tlog.critical(__FILE__, __LINE__, tlOss);
				}

				for(pugi::xml_node bubbleNode = questionNode.child("bubble"); bubbleNode; bubbleNode = bubbleNode.next_sibling("bubble")) {
					struct BubbleLayout currentBubble;

					//Set bubble name
					pugi::xml_attribute bubbleContentAttr = bubbleNode.attribute("content");
					if(bubbleContentAttr) {
						currentBubble.answer_ = bubbleContentAttr.value();
					} else {
						status = -1;
						tlOss << "Question " << currentQuestion.questionNumber_ << " in \"" << currentQuestionGroup.name_ << "\" has a bubble without a name";
						tlog.critical(__FILE__, __LINE__, tlOss);
					}
					
					//Set bubble x coord
					pugi::xml_attribute bubbleXcoordAttr = bubbleNode.attribute("x");
					if(bubbleXcoordAttr) {
						currentBubble.location_[0] = bubbleXcoordAttr.as_float(-1.0);
					} else {
						status = -1;
						tlOss << "Question " << currentQuestion.questionNumber_ << " in \"" << currentQuestionGroup.name_ << "\" has a bubble that does not specify its x coordinate";
						tlog.critical(__FILE__, __LINE__, tlOss);
					}

					//Set bubble y coord
					pugi::xml_attribute bubbleYcoordAttr = bubbleNode.attribute("y");
					if(bubbleYcoordAttr) {
						currentBubble.location_[1] = bubbleYcoordAttr.as_float(-1.0);
					} else {
						status = -1;
						tlOss << "Question " << currentQuestion.questionNumber_ << " in \"" << currentQuestionGroup.name_ << "\" has a bubble that does not specify its y coordinate";
						tlog.critical(__FILE__, __LINE__, tlOss);
					}

					//Set bubble radius
					pugi::xml_attribute bubbleRadiusAttr = bubbleNode.attribute("r");
					if(bubbleRadiusAttr) {
						currentBubble.location_[2] = bubbleRadiusAttr.as_float(-1.0);
					} else {
						status = -1;
						tlOss << "Question " << currentQuestion.questionNumber_ << " in \"" << currentQuestionGroup.name_ << "\" has a bubble that does not specify its radius";
						tlog.critical(__FILE__, __LINE__, tlOss);
					}

					//Add bubble to question
					currentQuestion.addBubble(currentBubble);
				}

				//Add question to group
				currentQuestionGroup.addQuestion(currentQuestion);
				i++;
			}

			//Add group to sheet
			questionGroups_.push_back(currentQuestionGroup);
		}
	}

	if(status >= 0) {
		tlOss << "Successfully parsed sheet layout from \"" << filename << "\"";
		tlog.info(__FILE__, __LINE__, tlOss);
	}

	return status;
}

int SheetLayout::save(const std::string & filename) const {
	return 0;
}

const std::vector<struct QuestionGroupLayout>& SheetLayout::getQuestionGroups() const {
	return questionGroups_;
}

const std::string& SheetLayout::getTitle() const {
	return title_;
}

std::ostream& operator<<(std::ostream& os, const SheetLayout& sheetLayout) {
	os << "Sheet layout template \"" << sheetLayout.getTitle() << "\"" << std::endl;
	for(QuestionGroupLayout group : sheetLayout.getQuestionGroups()) {
		os << "Question group \"" << group.name_ << "\"" << std::endl;
		for(QuestionLayout question : group.questions_) {
			os << "    Question " << question.questionNumber_ << std::endl;
			for(BubbleLayout bubble : question.bubbles_) {
				os << "        Bubble \"" << bubble.answer_ << "\" at " << bubble.location_  << std::endl;
			}
		}
	}
	return os;
}


