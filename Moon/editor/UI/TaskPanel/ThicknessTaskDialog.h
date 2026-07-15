#pragma once
#include "editor/UI/TaskPanel/ParamTaskDialog.h"
#include "editor/UI/TaskPanel/ShapeHelper.h"
namespace MOON {
	class ThicknessTaskDialog : public ParamTaskDialog, public ShapeHelper
	{
		
	public:
		explicit ThicknessTaskDialog(QWidget* parent = nullptr, Feature* feature = nullptr);
		~ThicknessTaskDialog();
		virtual QVariant getParamValue(const QString& propertyName)override;
		virtual void setParamValue(const QString& propertyName, const QVariant& value)override;
		virtual void clickOk() override;
		virtual void clickApply() override;
		virtual void clickCancel() override;
		virtual bool generateShape()override;
	private:
		void onWidgetLengthInvoke();
		class Internal;
		Internal* mInternal = nullptr;
	};
}