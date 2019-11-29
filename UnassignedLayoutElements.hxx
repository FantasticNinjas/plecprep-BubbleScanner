// 
// University of Massachusetts Amherst Physics Lecture Prep
//
// Project: ScantronReader
// File: UnassignedLayoutElements.hxx
// Author: Casey Massar (caseymassar@gmail.com)
// Description: Temporary container to hold sheet layout elements that have been created in the GUI but have not been assigned to a parent element.
//

#pragma once

#include <vector>
#include <QTreeWidgetItem>
#include <QList>

#include "SheetLayout.hxx"

class UnassignedLayoutElements {
public:
	UnassignedLayoutElements();
	~UnassignedLayoutElements();


	///
	/// <summary> Append a question group to the list of unassigned elements </summary>
	///
	/// <param name="questionGroupLayout"> The question group to add </param>
	///
	void add(const struct QuestionGroupLayout& questionGroupLayout);

	///
	/// <summary> Append a question to the list of unassigned elements </summary>
	///
	/// <param name="questionGroupLayout"> The question to add </param>
	///
	void add(const struct QuestionLayout& questionLayout);

	///
	/// <summary> Append a bubbble to the list of unassigned elements </summary>
	///
	/// <param name="questionGroupLayout"> The bubble to add </param>
	///
	void add(const struct BubbleLayout& bubbleLayout);



	///
	/// <summary> Get the number of question groups not assigned to any sheet layout </summary>
	///
	int numQuestionGroups() const;

	///
	/// <summary> Get the number of questions not assigned to any question group layout. Note that this only counts top level items </summary>
	///
	int numQuestions() const;

	///
	/// <summary> Get the number of bubbles not assigned to any question layout </summary>
	///
	int numBubbles() const;

	///
	/// <summary> Get the total number of layout elements not assigned to any sheet layout
	int numElements() const;



	///
	/// <summary> Retrieve a pointer to a specific question group layout by its absolute index </summary>
	///
	/// <param name="index"> The index of the question group to retrieve. Note that this must be a absolute index (i.e. the index of the question group 
	///                      amoung all unassigned layout items, not just other question groups. </param>
	///
	/// <returns> A pointer to the requested question group, or nullptr if the index is invalid </returns>
	///
	struct QuestionGroupLayout* getQuestionGroupLayout(int index);

	///
	/// <summary> Retrieve a pointer to a specific question layout by its absolute index </summary>
	///
	/// <param name="index"> The index of the question to retrieve. Note that this must be a absolute index (i.e. the index of the question 
	///                      amoung all unassigned layout items, not just other questions. </param>
	///
	/// <returns> A pointer to the requested question, or nullptr if the index is invalid </returns>
	///
	struct QuestionLayout* getQuestionLayout(int index);

	///
	/// <summary> Retrieve a pointer to a specific bubble layout by its absolute index </summary>
	///
	/// <param name="index"> The index of the bubble to retrieve. Note that this must be a absolute index (i.e. the index of the bubble 
	///                      amoung all unassigned layout items, not just other bubbles. </param>
	///
	/// <returns> A pointer to the requested bubble, or nullptr if the index is invalid </returns>
	///
	struct BubbleLayout* getBubbleLayout(int index);

	///
	/// <summary> Get a list of tree widget items representing all of the unassigned layout elements. </summary>
	///
	QList<QTreeWidgetItem *> treeWidgetItem();

private:
	//Storage for sheet layout items. Note it is not necessary to store side layouts as the GUI does not allow you to create side layouts that are not part of a sheet layout
	std::vector<struct QuestionGroupLayout> questionGroupLayouts_;
	std::vector<struct QuestionLayout> questionLayouts_;
	std::vector<struct BubbleLayout> bubbleLayouts_;

	///
	/// <summary> Get the relative index of a question group from its absolute index </summary>
	///
	/// <paraam name="absoluteIndex"> The global index of the question group (i.e. its index amoung all unassigned elements)
	///
	/// <returns> The relative index of the question group (i.e. its index amound other question groups), or a negative value if an invalid global index was supplied (or a valid global index that does not refer to a question grouop).</returns>
	///
	int questionGroupIndex(int absoluteIndex);

	///
	/// <summary> Get the relative index of a question from its absolute index </summary>
	///
	/// <paraam name="absoluteIndex"> The global index of the question (i.e. its index amoung all unassigned elements)
	///
	/// <returns> The relative index of the question (i.e. its index amound other questions), or a negative value if an invalid global index was supplied (or a valid global index that does not refer to a question).</returns>
	///
	int questionIndex(int absoluteIndex);

	///
	/// <summary> Get the relative index of a bubble from its absolute index </summary>
	///
	/// <paraam name="absoluteIndex"> The global index of the bubble (i.e. its index amoung all unassigned elements)
	///
	/// <returns> The relative index of the bubble (i.e. its index amound other bubbles), or a negative value if an invalid global index was supplied (or a valid global index that does not refer to a bubble).</returns>
	///
	int bubbleIndex(int absoluteIndex);
};

