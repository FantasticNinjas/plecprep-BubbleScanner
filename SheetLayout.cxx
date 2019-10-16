
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

	//Load sides of the sheet

	if(!sheetNode.child("side")) {
		status = 1;
		tlOss << "Sheet template \"" << title_ << "\" loaded from \"" << filename << "\" does not contain any side layouts.";
		tlog.warning(__FILE__, __LINE__, tlOss);
	}

	//Iterate over all question groups
	if(status == 0) {
		for(pugi::xml_node sideNode = sheetNode.child("side"); sideNode; sideNode = sideNode.next_sibling("side")) {
			struct SideLayout currentSide;

			//Set side number
			currentSide.sideNumber_ = sideLayouts_.size();

			//Add question groups to side layout
			for(pugi::xml_node groupNode = sideNode.child("group"); groupNode; groupNode = groupNode.next_sibling("group")) {

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

				for(pugi::xml_node questionNode = groupNode.child("question"); questionNode; questionNode = questionNode.next_sibling("question")) {
					struct QuestionLayout currentQuestion;

					currentQuestion.questionNumber_ = currentQuestionGroup.getNumQuestions();

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
				}

				//Add group to sheet
				currentSide.addQuestionGroup(currentQuestionGroup);
			}
			//Add side layout to sheet
			sideLayouts_.push_back(currentSide);
		}
	}

	if(status >= 0) {
		tlOss << "Successfully parsed sheet layout from \"" << filename << "\"";
		tlog.info(__FILE__, __LINE__, tlOss);
	}

	return status;
}

int SheetLayout::save(const std::string &filename) const {
	int status;

	pugi::xml_document doc;

	//Add sheet node
	pugi::xml_node sheetNode = doc.append_child("sheet");
	sheetNode.append_attribute("title") = title_.c_str();
	
	//Add nodes for each side of the sheet layout
	for(struct SideLayout side : sideLayouts_) {
		pugi::xml_node sideNode = sheetNode.append_child("side");

		//Add nodes for each question group
		for(struct QuestionGroupLayout group : side.questionGroups_) {
			pugi::xml_node groupNode = sideNode.append_child("group");
			//Set group name
			groupNode.append_attribute("name") = group.name_.c_str();

			//Set group content type
			switch(group.contentType_) {
			case QuestionGroupLayout::ContentType::PROBLEMS:
				groupNode.append_attribute("type") = "problems";
				break;
			case QuestionGroupLayout::ContentType::STRING:
				groupNode.append_attribute("type") = "string";
				break;
			case QuestionGroupLayout::ContentType::UNKNOWN:
				groupNode.append_attribute("type") = "unknown";

				tlOss << "The content type of group \"" << group.name_ << "\" of sheet \"" << title_ << "\" is \'Unknown\'. This may not be intended.";
				tlog.warning(__FILE__, __LINE__, tlOss);
				break;
			default:
				status = -1;
				tlOss << "Unrecognized content type found on group \"" << group.name_ << "\" of sheet \"" << title_ << "\"";
				tlog.critical(__FILE__, __LINE__, tlOss);
			}

			//Add questions
			for(struct QuestionLayout question : group.questions_) {
				pugi::xml_node questionNode = groupNode.append_child("question");
				
				//Add bubbles
				for(struct BubbleLayout bubble : question.bubbles_) {
					pugi::xml_node bubbleNode = questionNode.append_child("bubble");

					//Set bubble content
					bubbleNode.append_attribute("content") = bubble.answer_.c_str();
					//Set bubble x coordinate
					bubbleNode.append_attribute("x") = bubble.location_[0];
					//Set bubble y coordinate
					bubbleNode.append_attribute("y") = bubble.location_[1];
					//Set bubble radius
					bubbleNode.append_attribute("r") = bubble.location_[2];
				}
			}
		}
	}

	if(status >= 0) {
		if(doc.save_file(filename.c_str(), "    ")) {
			tlOss << "Successfully saved sheet layout \"" << title_ << "\" to \"" << filename << "\"";
			tlog.info(__FILE__, __LINE__, tlOss);
		} else {
			status = -1;
			tlOss << "Failed to write sheet layout \"" << title_ << "\" to \"" << filename << "\"";
			tlog.critical(__FILE__, __LINE__, tlOss);
		}
	}

	return status;
}

const std::vector<struct SideLayout>& SheetLayout::getSideLayouts() const {
	return sideLayouts_;
}

const std::string& SheetLayout::getTitle() const {
	return title_;
}

std::ostream& operator<<(std::ostream& os, const SheetLayout& sheetLayout) {
	os << "Sheet layout template \"" << sheetLayout.getTitle() << "\"" << std::endl;
	for(SideLayout side : sheetLayout.getSideLayouts()) {
		os << "Side " << side.sideNumber_ << std::endl;
		for(QuestionGroupLayout group : side.questionGroups_) {
			os << "    Question group \"" << group.name_ << "\"" << std::endl;
			for(QuestionLayout question : group.questions_) {
				os << "        Question " << question.questionNumber_ << std::endl;
				for(BubbleLayout bubble : question.bubbles_) {
					os << "            Bubble \"" << bubble.answer_ << "\" at " << bubble.location_ << std::endl;
				}
			}
		}
	}
	return os;
}


