#pragma once
#include "editor/UI/TaskPanel/BaseTaskDialog.h"
#include "editor/UI/TaskPanel/ShapeHelper.h"
namespace MOON {
	class SketcherObj;
	class PadTaskDialog : public BaseTaskDialog,public ShapeHelper
	{
		Q_OBJECT
	public:
		explicit PadTaskDialog(QWidget* parent = nullptr);
		virtual ~PadTaskDialog()override;
		virtual bool generatePreviewShape()override;
		virtual void buildUi() override;
		virtual void clickOk() override;
		virtual void clickApply() override;
		virtual void clickCancel() override;
		void onValueChange();
		void onAngleChange();
		void onLengthChange();
		
	private:
		void onWidgetLengthInvoke();
		void onWidgetAngleInvoke();
		class Internal;
		Internal* mInternal = nullptr;
	};
}