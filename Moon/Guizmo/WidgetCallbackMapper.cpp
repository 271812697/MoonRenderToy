#include "WidgetCallbackMapper.h"
#include "AbstractWidget.h"
#include "ExecuteCommand.h"
#include "EventData.h"
#include "WidgetEventTranslator.h"
#include <map>
#include "WidgetCallbackMapper.h"

namespace MOON {

	// Callbacks are stored as a pair of (Object,Method) in the map.
	struct vtkCallbackPair
	{
		vtkCallbackPair()
			: Widget(nullptr)
			, Callback(nullptr)
		{
		} // map requires empty constructor
		vtkCallbackPair(vtkAbstractWidget* w, vtkWidgetCallbackMapper::CallbackType f)
			: Widget(w)
			, Callback(f)
		{
		}

		vtkAbstractWidget* Widget;
		vtkWidgetCallbackMapper::CallbackType Callback;
	};

	// The map tracks the correspondence between widget events and callbacks
	class vtkCallbackMap : public std::map<unsigned long, vtkCallbackPair>
	{
	public:
		typedef vtkCallbackMap CallbackMapType;
		typedef std::map<unsigned long, vtkCallbackPair>::iterator CallbackMapIterator;
	};

	//------------------------------------------------------------------------------
	vtkWidgetCallbackMapper::vtkWidgetCallbackMapper()
	{
		this->CallbackMap = new vtkCallbackMap;
		this->EventTranslator = nullptr;
	}

	//------------------------------------------------------------------------------
	vtkWidgetCallbackMapper::~vtkWidgetCallbackMapper()
	{
		delete this->CallbackMap;
		if (this->EventTranslator)
		{
			//this->EventTranslator->Delete();
		}
	}

	vtkWidgetCallbackMapper* MOON::vtkWidgetCallbackMapper::New()
	{
		return nullptr;
	}

	//------------------------------------------------------------------------------
	void vtkWidgetCallbackMapper::SetEventTranslator(vtkWidgetEventTranslator* t)
	{
		if (this->EventTranslator != t)
		{
			if (this->EventTranslator)
			{
				//this->EventTranslator->Delete();
			}
			this->EventTranslator = t;
			if (this->EventTranslator)
			{
				//this->EventTranslator->Register(this);
			}

		}
	}

	//------------------------------------------------------------------------------
	void vtkWidgetCallbackMapper::SetCallbackMethod(
		unsigned long VTKEvent, unsigned long widgetEvent, vtkAbstractWidget* w, CallbackType f)
	{
		this->EventTranslator->SetTranslation(VTKEvent, widgetEvent);
		this->SetCallbackMethod(widgetEvent, w, f);
	}

	//------------------------------------------------------------------------------
	void vtkWidgetCallbackMapper::SetCallbackMethod(unsigned long VTKEvent, int modifier, char keyCode,
		int repeatCount, const char* keySym, unsigned long widgetEvent, vtkAbstractWidget* w,
		CallbackType f)
	{
		this->EventTranslator->SetTranslation(
			VTKEvent, modifier, keyCode, repeatCount, keySym, widgetEvent);
		this->SetCallbackMethod(widgetEvent, w, f);
	}

	//------------------------------------------------------------------------------
	void vtkWidgetCallbackMapper::SetCallbackMethod(unsigned long VTKEvent, vtkEventData* edata,
		unsigned long widgetEvent, vtkAbstractWidget* w, CallbackType f)
	{
		// make sure the type is set
		edata->SetType(VTKEvent);

		this->EventTranslator->SetTranslation(VTKEvent, edata, widgetEvent);
		this->SetCallbackMethod(widgetEvent, w, f);
	}

	//------------------------------------------------------------------------------
	void vtkWidgetCallbackMapper::SetCallbackMethod(
		unsigned long widgetEvent, vtkAbstractWidget* w, CallbackType f)
	{
		(*this->CallbackMap)[widgetEvent] = vtkCallbackPair(w, f);
	}

	//------------------------------------------------------------------------------
	void vtkWidgetCallbackMapper::InvokeCallback(unsigned long widgetEvent)
	{
		vtkCallbackMap::CallbackMapIterator iter = this->CallbackMap->find(widgetEvent);
		if (iter != this->CallbackMap->end())
		{
			vtkAbstractWidget* w = (*iter).second.Widget;
			CallbackType f = (*iter).second.Callback;
			(*f)(w);
		}
	}

}