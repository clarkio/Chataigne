/*
  ==============================================================================

    SimpleConversionFilters.cpp
    Created: 17 Apr 2018 10:25:04am
    Author:  Ben

  ==============================================================================
*/

#include "SimpleConversionFilters.h"

SimpleConversionFilter::SimpleConversionFilter(const String &name, var params, StringRef outTypeString, Multiplex* multiplex) :
	MappingFilter(name, params, multiplex),
	retargetComponent(nullptr),
	outTypeString(outTypeString)
{
	autoSetRange = false;
	retargetComponent = filterParams.addEnumParameter("Extract Component", "This is the component of the source parameter to extract");
	retargetComponent->setCustomShortName("retargetComponent");
	retargetComponent->forceSaveValue = true;
}

SimpleConversionFilter::~SimpleConversionFilter()
{
}

Parameter* SimpleConversionFilter::setupSingleParameterInternal(Parameter* source, int multiplexIndex)
{
	Parameter* p = (Parameter *)ControllableFactory::getInstance()->createControllable(outTypeString);
	p->setNiceName(source->niceName);

	if (multiplexIndex != 0) return p; //only setup for 1st if multiplex multiplex

	retargetComponent->clearOptions();

	if (p->isComplex() == source->isComplex())
	{
		if (source->hasRange() && p->canHaveRange) p->setRange(source->minimumValue, source->maximumValue);
		transferType = DIRECT;
		retargetComponent->setEnabled(false);
		retargetComponent->hideInEditor = true;
	}
	else
	{
		retargetComponent->setEnabled(true);
		retargetComponent->hideInEditor = false;
		transferType = p->isComplex() ? TARGET : EXTRACT;
		retargetComponent->setNiceName(transferType == TARGET ? "Target Component" : "Extract Component");
		Parameter * retargetP = transferType == TARGET ? p : source;
		StringArray valueNames = retargetP->getValuesNames();
		for (int i = 0; i < valueNames.size(); ++i)
		{
			retargetComponent->addOption(valueNames[i], i, true);
		}
	}

	return p;
}


MappingFilter::ProcessResult SimpleConversionFilter::processSingleParameterInternal(Parameter* source, Parameter* out, int multiplexIndex)
{
	switch (transferType)
	{
	case DIRECT:
	{
		if (!source->isComplex()) out->setValue(convertValue(source, source->getValue()));
		else
		{
			var val = var();
			var sourceVal = source->getValue();
			for (int i = 0; i < source->value.size() && i < out->value.size(); ++i) val.append(convertValue(source, sourceVal[i]));
			while (val.size() < out->value.size()) val.append(0);
			out->setValue(val);
		}
	}
	break;

	case EXTRACT: 
	{
		out->setValue(convertValue(source, source->value[(int)retargetComponent->getValueData()]));
		DBG("Extracted out value : " << out->floatValue());
	}
	break;

	case TARGET:
	{
		var val = var();
		for (int i = 0; i < out->value.size(); ++i) val.append(0);
		val[(int)retargetComponent->getValueData()] = convertValue(source, source->getValue());
		out->setValue(val);
	}
	break;

	default:
		return UNCHANGED;
	}

	return CHANGED;
}



ToFloatFilter::ToFloatFilter(var params, Multiplex* multiplex) :
	SimpleConversionFilter(getTypeString(), params, FloatParameter::getTypeStringStatic(), multiplex)
{
}

Parameter * ToFloatFilter::setupSingleParameterInternal(Parameter* source, int multiplexIndex)
{
	Parameter* p = SimpleConversionFilter::setupSingleParameterInternal(source, multiplexIndex);

	if (source->hasRange())
	{
		if (source->minimumValue.isArray())
		{
			int index = (int)retargetComponent->getValueData();
			p->setRange(source->minimumValue[index], source->maximumValue[index]);
		}
		else
		{
			p->setRange(source->minimumValue, source->maximumValue);
		}
	}

	return p;
}

var ToFloatFilter::convertValue(Parameter * source, var sourceValue)
{
	if (sourceValue.isString()) return sourceValue.toString().getFloatValue();
	return (float)sourceValue;
}


ToIntFilter::ToIntFilter(var params, Multiplex* multiplex) :
	SimpleConversionFilter(getTypeString(), params, IntParameter::getTypeStringStatic(), multiplex)
{
	modeParam = addEnumParameter("Mode", "Conversion mode");
	modeParam->addOption("Round", ROUND)->addOption("Floor", FLOOR)->addOption("Ceil", CEIL);
}

var ToIntFilter::convertValue(Parameter * source, var sourceValue)
{
	Mode m = modeParam->getValueDataAsEnum<Mode>();
	switch(m)
	{
	case ROUND: return roundf(source->floatValue());
	case FLOOR: return floorf(source->floatValue()); 
	case CEIL:  return ceilf(source->floatValue());
	}

	return 0;
}

ToStringFilter::ToStringFilter(var params, Multiplex* multiplex) :
	SimpleConversionFilter(getTypeString(), params, StringParameter::getTypeStringStatic(), multiplex)
{
	format = filterParams.addEnumParameter("Format", "The format of the string");
	format->addOption("Number", NUMBER)->addOption("Time", TIME);
	numDecimals = filterParams.addIntParameter("Number of Decimals", "Maximum number of decimals", 3, 0, 26);
	prefix = filterParams.addStringParameter("Prefix", "Something prepended to the result", "");
	suffix = filterParams.addStringParameter("Suffix", "Something appended  to the result", "");
}

var ToStringFilter::convertValue(Parameter * source, var sourceValue)
{
	String result = prefix->stringValue();

	if (!sourceValue.isArray())
	{
		if (sourceValue.isString()) result += sourceValue.toString();
		else
		{
			Format f = format->getValueDataAsEnum<Format>();
			switch (f)
			{
			case NUMBER:
				result += String((float)sourceValue, numDecimals->intValue());
				break;

			case TIME:
				result += StringUtil::valueToTimeString((float)sourceValue);
				break;
			}
		}
	}
	
	result += suffix->stringValue();
	return result;
}

void ToStringFilter::filterParamChanged(Parameter * p)
{
	numDecimals->hideInEditor = format->getValueDataAsEnum<Format>() != NUMBER;
	SimpleConversionFilter::filterParamChanged(p);
}

ToPoint2DFilter::ToPoint2DFilter(var params, Multiplex* multiplex) :
	SimpleConversionFilter(getTypeString(), params, Point2DParameter::getTypeStringStatic(), multiplex)
{
}

var ToPoint2DFilter::convertValue(Parameter * source, var sourceValue)
{
	if (sourceValue.isString()) return sourceValue.toString().getFloatValue();
	return (float)sourceValue;
}

ToPoint3DFilter::ToPoint3DFilter(var params, Multiplex* multiplex) :
	SimpleConversionFilter(getTypeString(), params, Point3DParameter::getTypeStringStatic(), multiplex)
{
}

var ToPoint3DFilter::convertValue(Parameter * source, var sourceValue)
{
	if (sourceValue.isString()) return sourceValue.toString().getFloatValue();
	return (float)sourceValue;
}

ToColorFilter::ToColorFilter(var params, Multiplex* multiplex) :
	SimpleConversionFilter(getTypeString(), params, ColorParameter::getTypeStringStatic(), multiplex),
	baseColor(nullptr)
{
}

ToColorFilter::~ToColorFilter()
{
}

Parameter* ToColorFilter::setupSingleParameterInternal(Parameter* sourceParam, int multiplexIndex)
{
	Parameter* p = SimpleConversionFilter::setupSingleParameterInternal(sourceParam, multiplexIndex);

	if (multiplexIndex != 0) return p; //only setup for 1st if multiplex


	if (transferType != TARGET)
	{
		if (baseColor != nullptr)
		{
			filterParams.removeControllable(baseColor);
			baseColor = nullptr;
		}
		
	}
	else if (baseColor == nullptr)
	{
		baseColor = filterParams.addColorParameter("Base Color", "Color to use to convert", Colours::red);
	}

	switch(transferType)
	{

	case TARGET:
		retargetComponent->addOption("Hue", HUE)->addOption("Saturation", SAT)->addOption("Brightness", VAL);
		retargetComponent->setValueWithData(HUE);
		if(baseColor != nullptr) baseColor->hideInEditor = false;
		break;
            
    default:
		if(baseColor != nullptr) baseColor->hideInEditor = true;
        break;

	}

	return p;
}

MappingFilter::ProcessResult ToColorFilter::processSingleParameterInternal(Parameter* source, Parameter* out, int multiplexIndex)
{
	if (source->value.isArray())
	{
		if (source->value.size() >= 3)
		{
			Colour c = Colour::fromFloatRGBA((float)source->value[0], (float)source->value[1], (float)source->value[2], source->value.size() > 3 ? (float)source->value[3] : 1.0f);
			((ColorParameter*)out)->setColor(c);
			return CHANGED;
		}

		return UNCHANGED; //don't process
	}

	//single value
	switch (transferType)
	{

	case TARGET:
	{
		int comp = retargetComponent->getValueData();

		switch (comp)
		{
		case HUE:
		{
			Colour c = baseColor->getColor().withHue(source->value);
			((ColorParameter*)out)->setColor(c);
		}
		break;

		case SAT:
		{
			Colour c = baseColor->getColor().withSaturation(source->value);
			((ColorParameter*)out)->setColor(c);
		}
		break;

		case VAL:
		{
			Colour c = baseColor->getColor().withBrightness(source->value);
			((ColorParameter*)out)->setColor(c);
		}
		break;

		default:
		{
			var val = var();
			for (int i = 0; i < out->value.size(); ++i) val.append(baseColor->value[i]); //force alpha to 1
			val[(int)retargetComponent->getValueData()] = convertValue(source, source->getValue());
			out->setValue(val);
		}
		break;
		}
		
	}
	break;

	default:
		return SimpleConversionFilter::processSingleParameterInternal(source, out, multiplexIndex);
		break;
	}

	return CHANGED;
}

ToBooleanFilter::ToBooleanFilter(var params, Multiplex* multiplex) :
	SimpleConversionFilter(getTypeString(), params, BoolParameter::getTypeStringStatic(), multiplex)
{
	toggleMode = addBoolParameter("Toggle Mode", "If checked, this will act as a toggle, and its value will be inverted when input value is 1", false);
}

MappingFilter::ProcessResult ToBooleanFilter::processSingleParameterInternal(Parameter* source, Parameter* out, int multiplexIndex)
{
	bool val = (source->value.isString() ?source->value.toString().getFloatValue():source->floatValue()) >= 1;
	if (toggleMode->boolValue())
	{
		if (val) out->setValue(!out->boolValue());
	}
	else out->setValue(val);

	return CHANGED;
}
