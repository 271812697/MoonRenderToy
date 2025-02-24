#pragma once
#include "GizmoObject.h"

namespace MOON {
	class vtkWidgetEvent;
	class vtkAbstractWidget;
	class vtkWidgetEventTranslator;
	class vtkCallbackMap; // PIMPL encapsulation of STL map
	class vtkEventData;

	class  vtkWidgetCallbackMapper : public GizmoObject
	{
	public:

		static vtkWidgetCallbackMapper* New();

		void SetEventTranslator(vtkWidgetEventTranslator* t);
		virtual vtkWidgetEventTranslator* GetEventTranslator()
		{
			return this->EventTranslator;
		};
		///@}

		typedef void (*CallbackType)(vtkAbstractWidget*);


		void SetCallbackMethod(
			unsigned long VTKEvent, unsigned long widgetEvent, vtkAbstractWidget* w, CallbackType f);
		void SetCallbackMethod(unsigned long VTKEvent, int modifiers, char keyCode, int repeatCount,
			const char* keySym, unsigned long widgetEvent, vtkAbstractWidget* w, CallbackType f);
		void SetCallbackMethod(unsigned long VTKEvent, vtkEventData* ed, unsigned long widgetEvent,
			vtkAbstractWidget* w, CallbackType f);

		void InvokeCallback(unsigned long widgetEvent);

	protected:
		vtkWidgetCallbackMapper();
		~vtkWidgetCallbackMapper() override;
		vtkWidgetEventTranslator* EventTranslator;
		vtkCallbackMap* CallbackMap;
		void SetCallbackMethod(unsigned long widgetEvent, vtkAbstractWidget* w, CallbackType f);

	private:
		vtkWidgetCallbackMapper(const vtkWidgetCallbackMapper&) = delete;
		void operator=(const vtkWidgetCallbackMapper&) = delete;
	};

}