/*
  ==============================================================================

    ScriptCommandEditor.cpp
    Created: 28 Jan 2019 8:38:36pm
    Author:  bkupe

  ==============================================================================
*/

#include "ScriptCommandEditor.h"

ScriptCommandEditor::ScriptCommandEditor(ScriptCommand * sc, bool isRoot) :
	GenericControllableContainerEditor(sc, isRoot,false),
	scriptCommand(sc)
{
	resetAndBuild();
}

ScriptCommandEditor::~ScriptCommandEditor()
{
}

void ScriptCommandEditor::clear()
{
	for (auto &ce : childEditors)
	{
		MappableParameterEditor * mpe = dynamic_cast<MappableParameterEditor *>(ce);
		if (mpe != nullptr) mpe->removeMappableListener(this);
	}

	GenericControllableContainerEditor::clear();
}

void ScriptCommandEditor::useForMappingChanged(MappableParameterEditor * mpe)
{
	WeakReference<Parameter> p = mpe->paramEditor->parameter;
	if (p == nullptr || p.wasObjectDeleted()) return;
	if (mpe->useForMapping.boolValue()) scriptCommand->addTargetMappingParameterAt(p, mpe->mappingIndex.intValue()-1); //-1 is to allow user to choose from 1 to .. instead of 0 to ..
 	else scriptCommand->removeTargetMappingParameter(p);
}

InspectableEditor * ScriptCommandEditor::getEditorUIForControllable(Controllable * c)
{
	if (scriptCommand->context != MAPPING || c->type == Controllable::TRIGGER || c->type == Controllable::ENUM || c->type == Controllable::TARGET) return GenericControllableContainerEditor::getEditorUIForControllable(c);
	
	Parameter * p = (Parameter *)c;
	MappableParameterEditor * mpe = new MappableParameterEditor(p, false);
	
	if (scriptCommand->parameterToIndexMap.contains(p))
	{
		mpe->useForMapping.setValue(true);
		mpe->mappingIndex.setValue(scriptCommand->parameterToIndexMap[p]);
	}

	mpe->addMappableListener(this);
	return mpe;
}

void ScriptCommandEditor::removeEditorUI(InspectableEditor * i, bool resize)
{
	MappableParameterEditor * mpe = dynamic_cast<MappableParameterEditor *>(i);
	if (mpe != nullptr) mpe->removeMappableListener(this);
	GenericControllableContainerEditor::removeEditorUI(i);
}
