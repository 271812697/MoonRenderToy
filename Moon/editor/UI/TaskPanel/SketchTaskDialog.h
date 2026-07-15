#pragma once
#include "editor/UI/TaskPanel/ParamTaskDialog.h"
#include "editor/UI/TaskPanel/ShapeHelper.h"
namespace MOON {
	class SketchTaskDialog : public ParamTaskDialog, public ShapeHelper
	{
		Q_OBJECT
	public:
		explicit SketchTaskDialog(QWidget* parent = nullptr,Feature* feature =nullptr);
		~SketchTaskDialog();
		virtual QVariant getParamValue(const QString& propertyName)override;
		virtual void setParamValue(const QString& propertyName, const QVariant& value)override;
		virtual void clickOk() override;
		virtual void clickApply() override;
		virtual void clickCancel() override;
		void onSelectPlane();
	private:
		class Internal;
		Internal* mInternal = nullptr;
	};
}