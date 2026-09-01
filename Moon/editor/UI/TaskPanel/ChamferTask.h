#pragma once
#include "editor/UI/TaskPanel/ParamTaskDialog.h"
#include "editor/UI/TaskPanel/ShapeHelper.h"
namespace MOON {
	class ChamferTask : public ParamTaskDialog, public ShapeHelper {
	public:
		explicit ChamferTask(QWidget* parent = nullptr, Feature* feature = nullptr);
		~ChamferTask();
		virtual QVariant getParamValue(const QString& propertyName);
		virtual void setParamValue(const QString& propertyName, const QVariant& value);
		virtual void clickOk() override;
		virtual void clickApply() override;
		virtual void clickCancel() override;
	private:
		void onWidgetLengthInvoke1();
		void onWidgetLengthInvoke2();
		class Internal;
		Internal* mInternal = nullptr;
	};
}
