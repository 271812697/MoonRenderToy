#pragma once
#include "editor/UI/TaskPanel/BaseTaskDialog.h"
#include "editor/UI/TaskPanel/ShapeHelper.h"
namespace MOON {
	enum ExtrudeType
	{
		Additive,
		Subtractive
	};
	class ExtrudeTaskDialog : public BaseTaskDialog,public ShapeHelper
	{
		Q_OBJECT
	public:
		explicit ExtrudeTaskDialog(QWidget* parent = nullptr, ExtrudeType type=Additive);
		virtual ~ExtrudeTaskDialog()override;
		virtual bool generateShape()override;
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