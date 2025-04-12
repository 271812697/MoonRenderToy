#pragma once
#include "GizmoObserver.h"

namespace MOON {
	class WidgetEventTranslator;
	class WidgetCallbackMapper;
	//class WidgetRepresentation;

	class  AbstractWidget
		: public InteractorObserver
	{
	public:
		void SetEnabled(int) override;
		virtual void SetProcessEvents(bool _arg)
		{

			if (this->ProcessEvents != _arg)
			{
				this->ProcessEvents = _arg;

			}
		}
		virtual bool GetProcessEventsMinValue() { return 0; }
		virtual bool GetProcessEventsMaxValue() { return 1; };
		virtual bool GetProcessEvents()
		{
			return this->ProcessEvents;
		}
		virtual void ProcessEventsOn() { this->SetProcessEvents(static_cast<bool>(1)); }
		virtual void ProcessEventsOff() { this->SetProcessEvents(static_cast<bool>(0)); };

		///@}

		/**
		 * Get the event translator. Careful manipulation of this class enables
		 * the user to override the default event bindings.
		 */
		WidgetEventTranslator* GetEventTranslator() { return this->EventTranslator; }

		/**
		 * Create the default widget representation if one is not set. The
		 * representation defines the geometry of the widget (i.e., how it appears)
		 * as well as providing special methods for manipulting the state and
		 * appearance of the widget.
		 */
		virtual void CreateDefaultRepresentation() = 0;

		/**
		 * This method is called by subclasses when a render method is to be
		 * invoked on the RenderWindowInteractor. This method should be called
		 * (instead of RenderWindow::Render() because it has built into it
		 * optimizations for minimizing renders and/or speeding renders.
		 */
		void Render();

		/**
		 * Specifying a parent to this widget is used when creating composite
		 * widgets. It is an internal method not meant to be used by the public.
		 * When a widget has a parent, it defers the rendering to the parent. It
		 * may also defer managing the cursor (see ManagesCursor ivar).
		 */
		void SetParent(AbstractWidget* parent) { this->Parent = parent; }
		virtual AbstractWidget* GetParent()
		{

			return this->Parent;
		};


		///@{
		/**
		 * Return an instance of WidgetRepresentation used to represent this
		 * widget in the scene. Note that the representation is a subclass of
		 * Prop (typically a subclass of WidgetRepresentation) so it can be
		 * added to the renderer independent of the widget.
		 */
		 //	WidgetRepresentation* GetRepresentation()
			 //{
			 //	this->CreateDefaultRepresentation();
			 //	return this->WidgetRep;
			 //}



		void SetPriority(float) override;

	protected:
		AbstractWidget();
		~AbstractWidget() override;

		// Handles the events; centralized here for all widgets.
		static void ProcessEventsHandler(
			GizmoObject* object, unsigned long event, void* clientdata, void* calldata);

		// The representation for the widget. This is typically called by the
		// SetRepresentation() methods particular to each widget (i.e. subclasses
		// of this class). This method does the actual work; the SetRepresentation()
		// methods constrain the type that can be set.
		//void SetWidgetRepresentation(WidgetRepresentation* r);
		//WidgetRepresentation* WidgetRep;



		// For translating and invoking events
		WidgetEventTranslator* EventTranslator;
		WidgetCallbackMapper* CallbackMapper;

		// The parent, if any, for this widget
		AbstractWidget* Parent;

		// Call data which can be retrieved by the widget. This data is set
		// by ProcessEvents() if call data is provided during a callback
		// sequence.
		void* CallData;

		// Flag indicating if the widget should handle interaction events.
		// On by default.
		bool ProcessEvents;


	private:
		AbstractWidget(const AbstractWidget&) = delete;
		void operator=(const AbstractWidget&) = delete;
	};

}