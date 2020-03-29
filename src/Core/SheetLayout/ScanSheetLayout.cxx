
#include <sstream>

#include "pugixml.hpp"

#include "ScanSheetLayout.hxx"
#include "TextLogging.hxx"

namespace {
	std::ostringstream tlOss;
	TextLogging tlog;
}

ScanSheetLayout::ScanSheetLayout() = default;
ScanSheetLayout::~ScanSheetLayout() = default;

void ScanSheetLayout::writeXml(std::ostream& os) {
	pugi::xml_document doc;

	//Add sheet node
	pugi::xml_node sheetNode = doc.append_child("sheet");

	//Set sheet title
	if(!title_.empty()) {
		sheetNode.append_attribute("title") = title_.c_str();
	}

	//Add nodes for each side of the sheet layout
	for(const auto& side : sheetSides_) {
		pugi::xml_node sideNode = sheetNode.append_child("side");
		if(!side.getReferenceImageFilename().empty()) {
			sideNode.append_attribute("bg-image") = side.getReferenceImageFilename().c_str();
		}

		//Add nodes for each question group
		for(size_t i = 0; i < side.numChildren(); i++) {
			const GroupLayout* group_ptr = side.groupAt(i);
			pugi::xml_node groupNode = sideNode.append_child("group");
			//Set group name
			groupNode.append_attribute("name") = group_ptr->getName().c_str();

			//Add questions
			for(size_t j = 0; j < group_ptr->numChildren(); j++) {
				const QuestionLayout* question_ptr = group_ptr->questionAt(j);

				pugi::xml_node questionNode = groupNode.append_child("question");

				questionNode.append_attribute("number") = question_ptr->getQuestionNumber();

				//Add bubbles
				for(size_t k = 0; k < question_ptr->numChildren(); k++) {
					const BubbleLayout* bubble_ptr = question_ptr->bubbleAt(k);

					pugi::xml_node bubbleNode = questionNode.append_child("bubble");

					//Set bubble content
					bubbleNode.append_attribute("content") = bubble_ptr->getAnswer().c_str();
					//Set bubble left bound
					bubbleNode.append_attribute("left") = bubble_ptr->getLeftEdge();
					//Set bubble top bound
					bubbleNode.append_attribute("top") = bubble_ptr->getTopEdge();
					//Set bubble right bound
					bubbleNode.append_attribute("right") = bubble_ptr->getRightEdge();
					//Set bubble bottom bound
					bubbleNode.append_attribute("bottom") = bubble_ptr->getBottomEdge();
				}
			}
		}
	}

	//Write the XML to the provided output stream
	//The four spaces in quotes specifies how lines will be indented
	doc.save(os, "    ");
}

int ScanSheetLayout::readXml(std::istream& is) {
	int status = 0;

	//Reset layout (in case this instance is being reused)
	reset();

	//Parse XML stream

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load(is);
	if(result) {
		tlOss << "Successfully parsed XML from stream";
		tlog.debug(__FILE__, __LINE__, tlOss);
	} else {
		status = -1;
		tlOss << "Failed to parse XML from input stream. PugiXML error message: " << result.description();
		tlog.critical(__FILE__, __LINE__, tlOss);
	}

	//Load sheet title

	pugi::xml_node sheetNode;
	if(status >= 0) {
		sheetNode = doc.first_child();
		if(std::string(sheetNode.name()) != "sheet") {
			status = -1;
			tlOss << "Failed to read sheet layout. XML document does not appear to be a template. Root node type: \"" << sheetNode.name() << "\"";
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
			tlOss << "XML sheet layout does not have a name.";
			tlog.warning(__FILE__, __LINE__, tlOss);
		}
	}

	//Iterate over all of the side layouts on this sheet in the XML

	if(status >= 0 && !sheetNode.child("side")) {
		status = 1;
		tlOss << "XML sheet layout does not contain any side layouts.";
		tlog.warning(__FILE__, __LINE__, tlOss);
	}

	if(status == 0) {
		for(pugi::xml_node sideNode = sheetNode.child("side"); sideNode; sideNode = sideNode.next_sibling("side")) {

			//Create a side layout with the appropriate side number
			SideLayout currentSide(sheetSides_.size());

			//Set background image (if one is specified)
			pugi::xml_attribute bgImageAttr;
			bgImageAttr = sideNode.attribute("bg-image");
			if(bgImageAttr) {
				currentSide.setReferenceImageFilename(bgImageAttr.value());
			}

			//Add question groups to side layout
			for(pugi::xml_node groupNode = sideNode.child("group"); groupNode; groupNode = groupNode.next_sibling("group")) {

				//Create a new QuestionGroupLayout to be populated and then added to the SheetLayout.
				GroupLayout currentQuestionGroup;

				//Set question group name
				pugi::xml_attribute groupNameAttr = groupNode.attribute("name");
				if(groupNameAttr) {
					currentQuestionGroup.setName(groupNameAttr.value());
				} else {
					status = 1;
					tlOss << "Encountered a question group without a name attribute in XML sheet layout \"" << title_ << "\"";
					tlog.warning(__FILE__, __LINE__, tlOss);
				}

				//Iterate over all of the questions on this group in the XML and add a question layout for each

				if(!groupNode.child("question")) {
					tlOss << "Question group \"" << currentQuestionGroup.getName() << "\" does not contain any questions.";
					tlog.warning(__FILE__, __LINE__, tlOss);
				}

				for(pugi::xml_node questionNode = groupNode.child("question"); questionNode; questionNode = questionNode.next_sibling("question")) {
					QuestionLayout currentQuestion;

					//If the question number is specified, use that. If not, it should be 1 more than the previous one in the group, unless this is the first question in the group, in which case it should be 0.
					pugi::xml_attribute questionNumberAttr = questionNode.attribute("number");
					if(questionNumberAttr) {
						currentQuestion.setQuestionNumber(questionNumberAttr.as_int());
					} else {
						if(currentQuestionGroup.numChildren() == 0) {
							currentQuestion.setQuestionNumber(1);
						} else {
							//Note that, because the questions in the XML are always sorted, the previous question will always have the current max question number.
							currentQuestion.setQuestionNumber(currentQuestionGroup.maxQuestionNumber() + 1);
						}
					}

					//Iterate over all of the bubbles on this question in the XML and add a bubble layout for each

					if(!questionNode.child("bubble")) {
						status = 1;
						tlOss << "Question " << currentQuestion.getQuestionNumber() << " in \"" << currentQuestionGroup.getName() << "\" does not contain any bubbles.";
						tlog.critical(__FILE__, __LINE__, tlOss);
					}

					for(pugi::xml_node bubbleNode = questionNode.child("bubble"); bubbleNode; bubbleNode = bubbleNode.next_sibling("bubble")) {
						struct BubbleLayout currentBubble;

						//Set bubble name
						pugi::xml_attribute bubbleContentAttr = bubbleNode.attribute("content");
						if(bubbleContentAttr) {
							currentBubble.setAnswer(bubbleContentAttr.value());
						} else {
							status = -1;
							tlOss << "Question " << currentQuestion.getQuestionNumber() << " in \"" << currentQuestionGroup.getName() << "\" has a bubble that does not specify its answer";
							tlog.critical(__FILE__, __LINE__, tlOss);
						}

						//Set bubble left bound
						pugi::xml_attribute bubbleLeftAttr = bubbleNode.attribute("left");
						if(bubbleLeftAttr) {
							currentBubble.setLeftEdge(bubbleLeftAttr.as_float(-1.0));
						} else {
							status = -1;
							tlOss << "Question " << currentQuestion.getQuestionNumber() << " in \"" << currentQuestionGroup.getName() << "\" has a bubble that does not specify its left coordinate";
							tlog.critical(__FILE__, __LINE__, tlOss);
						}

						//Set bubble top bound
						pugi::xml_attribute bubbleTopAttr = bubbleNode.attribute("top");
						if(bubbleTopAttr) {
							currentBubble.setTopEdge(bubbleTopAttr.as_float(-1.0));
						} else {
							status = -1;
							tlOss << "Question " << currentQuestion.getQuestionNumber() << " in \"" << currentQuestionGroup.getName() << "\" has a bubble that does not specify its top coordinate";
							tlog.critical(__FILE__, __LINE__, tlOss);
						}

						//Set bubble right bound
						pugi::xml_attribute bubbleRightAttr = bubbleNode.attribute("right");
						if(bubbleRightAttr) {
							currentBubble.setRightEdge(bubbleRightAttr.as_float(-1.0));
						} else {
							status = -1;
							tlOss << "Question " << currentQuestion.getQuestionNumber() << " in \"" << currentQuestionGroup.getName() << "\" has a bubble that does not specify its right coordinate";
							tlog.critical(__FILE__, __LINE__, tlOss);
						}

						//Set bubble bottom bound
						pugi::xml_attribute bubbleBottomAttr = bubbleNode.attribute("bottom");
						if(bubbleBottomAttr) {
							currentBubble.setBottomEdge(bubbleBottomAttr.as_float(-1.0));
						} else {
							status = -1;
							tlOss << "Question " << currentQuestion.getQuestionNumber() << " in \"" << currentQuestionGroup.getName() << "\" has a bubble that does not specify its bottom coordinate";
							tlog.critical(__FILE__, __LINE__, tlOss);
						}

						//Add bubble to question
						currentQuestion.addBubble(&currentBubble);
					}

					//Add question to group
					currentQuestionGroup.addQuestion(&currentQuestion);
				}

				//Add group to sheet
				currentSide.addGroup(&currentQuestionGroup);
			}
			//Add side layout to sheet
			sheetSides_.push_back(currentSide);
		}
	}

	if(status >= 0) {
		tlOss << "Successfully parsed sheet layout from XML";
		tlog.info(__FILE__, __LINE__, tlOss);
	}

	return status;
}

void ScanSheetLayout::reset() {
	title_ = "";
	sheetSides_.clear();
}

SideLayout* ScanSheetLayout::sideLayout(int sideNumber) {
	SideLayout* side = nullptr;
	if(sideNumber < numSides()) {
		side = &sheetSides_[sideNumber];
	}
	return side;
}

void ScanSheetLayout::newSide() {
	sheetSides_.push_back(SideLayout(numSides()));
}

size_t ScanSheetLayout::numSides() {
	return sheetSides_.size();
}

const std::string & ScanSheetLayout::getTitle() {
	return title_;
}

void ScanSheetLayout::setTitle(const std::string & title) {
	title_ = title;
}

ScanSheetLayout::iterator ScanSheetLayout::begin() {
	return ScanSheetLayout::iterator(*this, 0, -1, -1, -1);
}

ScanSheetLayout::iterator ScanSheetLayout::end() {
	return ScanSheetLayout::iterator(*this, numSides(), -1, -1, -1);
}


///////////////////////////////////////////////////////
//     Definitions for ScanSheetLayout::iterator     //
///////////////////////////////////////////////////////

ScanSheetLayout::iterator::iterator(ScanSheetLayout& scanSheet, int sideIndex, int groupIndex, int questionIndex, int bubbleIndex) :
	scanSheet_(scanSheet), sideIndex_(sideIndex), groupIndex_(groupIndex), questionIndex_(questionIndex), bubbleIndex_(bubbleIndex) {
}

SheetLayoutElement& ScanSheetLayout::iterator::operator*() {
	return *target_;
}

const SheetLayoutElement& ScanSheetLayout::iterator::operator*() const {
	return *target_;
}

SheetLayoutElement* ScanSheetLayout::iterator::operator->() {
	return target_;
}

const SheetLayoutElement* ScanSheetLayout::iterator::operator->() const {
	return target_;
}

ScanSheetLayout::iterator ScanSheetLayout::iterator::operator++(int) {
	ScanSheetLayout::iterator old = *this;
	++*this;
	return old;
}

ScanSheetLayout::iterator& ScanSheetLayout::iterator::operator++() {
	//A negative index indicates that this iterator points to the parent of that level. e.g. if bubbleIndex is negative but questionIndex is not, this iterator points to a questionIndex.

	if(sideIndex_ >= scanSheet_.numSides()) {
		//If the iterator is already past the end of the tree, don't keep advancing it.
		tlOss << "Tried to advance scan sheet layout iterator that is already at the end of the layout tree";
		tlog.debug(__FILE__, __LINE__, tlOss);
	} else if(groupIndex_ < 0) {
		//iterator points to a side layout, and should now point to its first question group
		groupIndex_ = 0;
	} else if(questionIndex_ < 0) {
		//Iterator points to a group layout, and should now point to its first question
		questionIndex_ = 0;
	} else if(bubbleIndex_ < 0) {
		//Iterator points to a question layout, and should now point to its first bubble
		bubbleIndex_ = 0;
	} else {
		//Iterator points to a bubble layout, and should now point to the next bubble
		bubbleIndex_++;
	}

	//If any indeces have overflowed (e.g. if bubbleIndex is greater than the number of bubbles in the current QuestionLayout), reset them and advance the next index.
	//Note: checking that the parent index is not negative first prevents accessing a null parent. Checking that the current index is not negative prevents accessing a parent before the parent index
	//      has been checked for overflow; this prevents accessing a null parent if there are no parents. (e.g. questionIndex was just set to 0, but this group has no questions)

	//Check if bubbleIndex has overflowed
	if(questionIndex_ >= 0 && bubbleIndex_ >= 0 && bubbleIndex_ >= scanSheet_.sideLayout(sideIndex_)->groupAt(groupIndex_)->questionAt(questionIndex_)->numChildren()) {
		bubbleIndex_ = -1;
		questionIndex_++;
	}

	//Check if questionIndex has overflowed
	if(groupIndex_ >= 0 && questionIndex_ >= 0 && questionIndex_ >= scanSheet_.sideLayout(sideIndex_)->groupAt(groupIndex_)->numChildren()) {
		bubbleIndex_ = -1;
		questionIndex_ = -1;
		groupIndex_++;
	}

	//Check if groupIndex has overflowed. sideIndex is never negative
	if(groupIndex_ >= 0 && groupIndex_ >= scanSheet_.sideLayout(sideIndex_)->numChildren()) {
		bubbleIndex_ = -1;
		questionIndex_ = -1;
		groupIndex_ = -1;
		sideIndex_++;
	}

	//Overflows of sideIndex are left alone; an iterator with a sideIndex 1 past the last one is intended to be used as the "end" iterator

	//Set target to the appropriate value
	if(sideIndex_ >= scanSheet_.numSides()) {
		//Iterator has reached end, target is null
		target_ = nullptr;
	} else if(groupIndex_ < 0) {
		//Iterator now points to a side layout
		target_ = scanSheet_.sideLayout(sideIndex_);
	} else if(questionIndex_ < 0) {
		//Iterator now points to a group layout
		target_ = scanSheet_.sideLayout(sideIndex_)->groupAt(groupIndex_);
	} else if(bubbleIndex_ < 0) {
		//Iterator now points to a question layout
		target_ = scanSheet_.sideLayout(sideIndex_)->groupAt(groupIndex_)->questionAt(questionIndex_);
	} else {
		//Iterator now points to a bubble layout
		target_ = scanSheet_.sideLayout(sideIndex_)->groupAt(groupIndex_)->questionAt(questionIndex_)->bubbleAt(bubbleIndex_);
	}
	return *this;
}

bool operator==(const ScanSheetLayout::iterator& lhs, const ScanSheetLayout::iterator& rhs) {
	return &lhs.scanSheet_ == &rhs.scanSheet_ &&
		lhs.sideIndex_ == rhs.sideIndex_ &&
		lhs.groupIndex_ == rhs.groupIndex_ &&
		lhs.questionIndex_ == rhs.questionIndex_ &&
		lhs.bubbleIndex_ == lhs.bubbleIndex_;
}

bool operator!=(const ScanSheetLayout::iterator& lhs, const ScanSheetLayout::iterator& rhs) {
	return !(lhs == rhs);
}
