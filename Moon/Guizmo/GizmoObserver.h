#pragma once
#include "GizmoObject.h"

namespace MOON {

    class  vtkRenderWindowInteractor;
    class  CallbackCommand;
    class  vtkInteractorObserver : public GizmoObject
    {
    public:

        virtual void SetInteractor(vtkRenderWindowInteractor* iren);
		virtual vtkRenderWindowInteractor* GetInteractor()
		{

			return this->Interactor;
		};
 
        virtual void SetEnabled(int) {}
        int GetEnabled() { return this->Enabled; }
        void EnabledOn() { this->SetEnabled(1); }
        void EnabledOff() { this->SetEnabled(0); }
        void On() { this->SetEnabled(1); }
        void Off() { this->SetEnabled(0); }

        virtual void SetPriority(float _arg)
        {

            if (this->Priority != (_arg < 0.0f ? 0.0f : (_arg > 1.0f ? 1.0f : _arg)))
            {
                this->Priority = (_arg < 0.0f ? 0.0f : (_arg > 1.0f ? 1.0f : _arg));

            }
        }
		virtual float GetPriority()
		{

			return this->Priority;
		}

		virtual void SetKeyPressActivation(bool _arg)
		{

			if (this->KeyPressActivation != _arg)
			{
				this->KeyPressActivation = _arg;
				
			}
		};
		virtual bool GetKeyPressActivation()
		{

			return this->KeyPressActivation;
		};
		virtual void KeyPressActivationOn() { this->SetKeyPressActivation(true); }
		virtual void KeyPressActivationOff()
		{
			this->SetKeyPressActivation(false);
		};

		virtual void SetKeyPressActivationValue(char _arg)
		{

			if (this->KeyPressActivationValue != _arg)
			{
				this->KeyPressActivationValue = _arg;
			}
		};
		virtual char GetKeyPressActivationValue()
		{

			return this->KeyPressActivationValue;
		};



        virtual void OnChar();



        void GrabFocus(ExecuteCommand* mouseEvents, ExecuteCommand* keypressEvents = nullptr);
        void ReleaseFocus();
        ///@}

    protected:
        vtkInteractorObserver();
        ~vtkInteractorObserver();

        ///@{
        /**
         * Utility routines used to start and end interaction.
         * For example, it switches the display update rate. It does not invoke
         * the corresponding events.
         */
        virtual void StartInteraction();
        virtual void EndInteraction();
        ///@}

        /**
         * Handles the char widget activation event. Also handles the delete event.
         */
        static void ProcessEvents(
            GizmoObject* object, unsigned long event, void* clientdata, void* calldata);

        ///@{
        /**
         * Helper method for subclasses.
         */
        void ComputeDisplayToWorld(double x, double y, double z, double worldPt[4]);
        void ComputeWorldToDisplay(double x, double y, double z, double displayPt[3]);
        ///@}

        // The state of the widget, whether on or off (observing events or not)
        int Enabled;

        // Used to process events
        CallbackCommand* EventCallbackCommand;    // subclasses use one
        CallbackCommand* KeyPressCallbackCommand; // listens to key activation

        // Priority at which events are processed
        float Priority;

        // This variable controls whether the picking is managed by the Picking
        // Manager process or not. True by default.
        bool PickingManaged;

        /**
         * Register internal Pickers in the Picking Manager.
         * Must be reimplemented by concrete widgets to register
         * their pickers.
         */
        virtual void RegisterPickers();

        /**
         * Unregister internal pickers from the Picking Manager.
         */
        void UnRegisterPickers();




        // Keypress activation controls
        bool KeyPressActivation;
        char KeyPressActivationValue;

        // Used to associate observers with the interactor
        vtkRenderWindowInteractor* Interactor;



        unsigned long CharObserverTag;
        unsigned long DeleteObserverTag;


        int RequestCursorShape(int requestedShape);

    private:
        vtkInteractorObserver(const vtkInteractorObserver&) = delete;
        void operator=(const vtkInteractorObserver&) = delete;
    };

}