#pragma once
#include "AbstractWidget.h"


namespace MOON {

	class vtkBoxWidget2 : public vtkAbstractWidget
	{
	public:

		static vtkBoxWidget2* New();


		void CreateDefaultRepresentation() override;

		/**
		 * Override superclasses' SetEnabled() method because the line
		 * widget must enable its internal handle widgets.
		 */
		void SetEnabled(int enabling) override;

	protected:
		vtkBoxWidget2();
		~vtkBoxWidget2() override;

		// Manage the state of the widget
		int WidgetState;
		enum WidgetStateType
		{
			Start = 0,
			Active
		};

		// These methods handle events
		static void SelectAction(vtkAbstractWidget*);
		static void EndSelectAction(vtkAbstractWidget*);
		static void TranslateAction(vtkAbstractWidget*);
		static void ScaleAction(vtkAbstractWidget*);
		static void MoveAction(vtkAbstractWidget*);
		static void SelectAction3D(vtkAbstractWidget*);
		static void EndSelectAction3D(vtkAbstractWidget*);
		static void MoveAction3D(vtkAbstractWidget*);
		static void StepAction3D(vtkAbstractWidget*);

		// Control whether scaling, rotation, and translation are supported
		bool TranslationEnabled;
		bool ScalingEnabled;
		bool RotationEnabled;
		bool MoveFacesEnabled;

		CallbackCommand* KeyEventCallbackCommand;
		static void ProcessKeyEvents(GizmoObject*, unsigned long, void*, void*);

	private:
		vtkBoxWidget2(const vtkBoxWidget2&) = delete;
		void operator=(const vtkBoxWidget2&) = delete;
	};
}