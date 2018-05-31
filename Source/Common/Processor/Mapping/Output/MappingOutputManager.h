/*
  ==============================================================================

    MappingOutputManager.h
    Created: 28 Oct 2016 8:11:54pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef MAPPINGOUTPUTMANAGER_H_INCLUDED
#define MAPPINGOUTPUTMANAGER_H_INCLUDED


#include "MappingOutput.h"

class MappingOutputManager :
	public BaseManager<MappingOutput>
{
public:
	MappingOutputManager();
	~MappingOutputManager();

	Parameter * outParam;

	void setValue(var value);

	void setOutParam(Parameter * p);

	InspectableEditor * getEditor(bool isRoot) override;

	class OutputManagerEvent {
	public:
		enum Type { OUTPUT_CHANGED };
		OutputManagerEvent(Type type) : type(type) {}
		Type type;
	};
	QueuedNotifier<OutputManagerEvent> omAsyncNotifier;
	typedef QueuedNotifier<OutputManagerEvent>::Listener AsyncListener;

	void addAsyncOutputManagerListener(AsyncListener* newListener) { omAsyncNotifier.addListener(newListener); }
	void addAsyncCoalescedOutputManagerListener(AsyncListener* newListener) { omAsyncNotifier.addAsyncCoalescedListener(newListener); }
	void removeAsyncOutputManagerListener(AsyncListener* listener) { omAsyncNotifier.removeListener(listener); }

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MappingOutputManager)

};




#endif  // MAPPINGOUTPUTMANAGER_H_INCLUDED
