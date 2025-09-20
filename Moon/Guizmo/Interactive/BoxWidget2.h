#pragma once
#include "AbstractWidget.h"


namespace MOON {

	class BoxWidget2 : public AbstractWidget
	{
	public:

		static BoxWidget2* New();


		void CreateDefaultRepresentation() override;

		/**
		 * Override superclasses' SetEnabled() method because the line
		 * widget must enable its internal handle widgets.
		 */
		void SetEnabled(int enabling) override;

	protected:
		BoxWidget2();
		~BoxWidget2() override;

		// Manage the state of the widget
		int WidgetState;
		enum WidgetStateType
		{
			Start = 0,
			Active
		};

		// These methods handle events
		static void SelectAction(AbstractWidget*);
		static void EndSelectAction(AbstractWidget*);
		static void TranslateAction(AbstractWidget*);
		static void ScaleAction(AbstractWidget*);
		static void MoveAction(AbstractWidget*);
		static void SelectAction3D(AbstractWidget*);
		static void EndSelectAction3D(AbstractWidget*);
		static void MoveAction3D(AbstractWidget*);
		static void StepAction3D(AbstractWidget*);

		// Control whether scaling, rotation, and translation are supported
		bool TranslationEnabled;
		bool ScalingEnabled;
		bool RotationEnabled;
		bool MoveFacesEnabled;

		CallbackCommand* KeyEventCallbackCommand;
		static void ProcessKeyEvents(GizmoObject*, unsigned long, void*, void*);

	private:
		BoxWidget2(const BoxWidget2&) = delete;
		void operator=(const BoxWidget2&) = delete;
	};
}