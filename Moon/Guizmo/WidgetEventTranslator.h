#pragma once

#include "GizmoObject.h"

namespace MOON {
	class vtkEventMap;
	class vtkRenderWindowInteractor;
	class CallbackCommand;
	class vtkEvent;
	class vtkAbstractWidget;
	class vtkEventData;

	// This is a lightweight class that should be used internally by the widgets
	class vtkWidgetEventTranslator : public GizmoObject
	{
	public:
		/**
		 * Instantiate the object.
		 */
		static vtkWidgetEventTranslator* New();


		///@{
		/**
		 * Use these methods to create the translation from a VTK event to a widget
		 * event. Specifying vtkWidgetEvent::NoEvent or an empty
		 * string for the (toEvent) erases the mapping for the event.
		 */
		void SetTranslation(unsigned long VTKEvent, unsigned long widgetEvent);
		void SetTranslation(const char* VTKEvent, const char* widgetEvent);
		void SetTranslation(unsigned long VTKEvent, int modifier, char keyCode, int repeatCount,
			const char* keySym, unsigned long widgetEvent);
		void SetTranslation(vtkEvent* VTKevent, unsigned long widgetEvent);
		void SetTranslation(unsigned long VTKEvent, vtkEventData* edata, unsigned long widgetEvent);
		///@}

		///@{
		/**
		 * Translate a VTK event into a widget event. If no event mapping is found,
		 * then the methods return vtkWidgetEvent::NoEvent or a nullptr string.
		 */
		unsigned long GetTranslation(unsigned long VTKEvent);
		const char* GetTranslation(const char* VTKEvent);
		unsigned long GetTranslation(
			unsigned long VTKEvent, int modifier, char keyCode, int repeatCount, const char* keySym);
		unsigned long GetTranslation(unsigned long VTKEvent, vtkEventData* edata);
		unsigned long GetTranslation(vtkEvent* VTKEvent);
		///@}

		///@{
		/**
		 * Remove translations for a binding.
		 * Returns the number of translations removed.
		 */
		int RemoveTranslation(
			unsigned long VTKEvent, int modifier, char keyCode, int repeatCount, const char* keySym);
		int RemoveTranslation(vtkEvent* e);
		int RemoveTranslation(vtkEventData* e);
		int RemoveTranslation(unsigned long VTKEvent);
		int RemoveTranslation(const char* VTKEvent);
		///@}

		/**
		 * Clear all events from the translator (i.e., no events will be
		 * translated).
		 */
		void ClearEvents();

		///@{
		/**
		 * Add the events in the current translation table to the interactor.
		 */
		void AddEventsToParent(vtkAbstractWidget*, CallbackCommand*, float priority);
		void AddEventsToInteractor(vtkRenderWindowInteractor*, CallbackCommand*, float priority);
		///@}

	protected:
		// Constructors/destructors made public for widgets to use
		vtkWidgetEventTranslator();
		~vtkWidgetEventTranslator() override;

		// Map VTK events to widget events
		vtkEventMap* EventMap;

		// Used for performance reasons to avoid object construction/deletion
		vtkEvent* Event;

	private:
		vtkWidgetEventTranslator(const vtkWidgetEventTranslator&) = delete;
		void operator=(const vtkWidgetEventTranslator&) = delete;
	};

}