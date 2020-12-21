/*
  ==============================================================================

	StandardConditionEditor.cpp
	Created: 28 Oct 2016 8:07:05pm
	Author:  bkupe

  ==============================================================================
*/

#include "StandardConditionEditor.h"

StandardConditionEditor::StandardConditionEditor(StandardCondition* _condition, bool isRoot) :
	ConditionEditor(_condition, isRoot),
	standardCondition(_condition)
{
	targetUI.reset(standardCondition->sourceTarget->getEditor(false));
	addChildComponent(targetUI.get());

	alwaysTriggerUI.reset(standardCondition->alwaysTrigger->createToggle());
	addAndMakeVisible(alwaysTriggerUI.get());
	
	toggleModeUI.reset(standardCondition->toggleMode->createToggle(ImageCache::getFromMemory(BinaryData::toggle_png, BinaryData::toggle_pngSize)));

	addAndMakeVisible(toggleModeUI.get());

	targetUI->setVisible(!standardCondition->editorIsCollapsed);

	setSize(100, 50);
	updateUI();
}

StandardConditionEditor::~StandardConditionEditor()
{
}

void StandardConditionEditor::setCollapsed(bool value, bool force, bool animate, bool doNotRebuild)
{
	ConditionEditor::setCollapsed(value, force, animate, doNotRebuild);

	targetUI->setVisible(!standardCondition->editorIsCollapsed);
	//if (sourceFeedbackUI != nullptr)	sourceFeedbackUI->setVisible(!standardCondition->editorIsCollapsed);
	if (comparatorUI != nullptr) comparatorUI->setVisible(!standardCondition->editorIsCollapsed);
}

void StandardConditionEditor::resetAndBuild()
{
	// do nothing
	//DBG("Here reset and build");
}

void StandardConditionEditor::resizedInternalHeaderItemInternal(Rectangle<int> & r)
{
	if (sourceFeedbackUI != nullptr)
	{
		sourceFeedbackUI->setBounds(r.removeFromRight(jmin(r.getWidth() - 150, 250)).reduced(2));
		r.removeFromRight(2);
	}
	BaseItemEditor::resizedInternalHeaderItemInternal(r);
}

void StandardConditionEditor::resizedInternalContent(Rectangle<int> & r)
{
	//ConditionEditor::resizedInternalContent(r);

	Rectangle<int> sr = r.withHeight(16).reduced(2, 0);
	targetUI->setBounds(sr);
	r.translate(0, 18);

	int ch = comparatorUI != nullptr ? comparatorUI->getHeight() : 16;
	Rectangle<int> cr = r.withHeight(ch);
	
	toggleModeUI->setBounds(cr.removeFromLeft(16).withHeight(16));
	cr.removeFromLeft(2);

	alwaysTriggerUI->setBounds(cr.removeFromRight(95).withHeight(16));
	cr.removeFromRight(2);

	if (comparatorUI != nullptr) comparatorUI->setBounds(cr);

	r.translate(0, ch + 2);
	r.setHeight(0);
}

void StandardConditionEditor::updateUI()
{
	if (sourceFeedbackUI != nullptr) removeChildComponent(sourceFeedbackUI.get());
	if (standardCondition->sourceControllables.size() > 0 && standardCondition->sourceControllables[0] != nullptr)
	{
		sourceFeedbackUI.reset(standardCondition->sourceControllables[0]->createDefaultUI());
		//sourceFeedbackUI->setForceFeedbackOnly(true);
		addAndMakeVisible(sourceFeedbackUI.get());
	}

	if (comparatorUI != nullptr)
	{
		removeChildComponent(comparatorUI.get());
		comparatorUI = nullptr;
	}
	if (standardCondition->comparator != nullptr)
	{
		comparatorUI.reset(standardCondition->comparator->createUI());
		addChildComponent(comparatorUI.get());
		comparatorUI->setVisible(!standardCondition->editorIsCollapsed);
	}

	resized();
}