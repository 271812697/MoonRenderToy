#pragma once
#include "editor/UI/TaskPanel/ParamTaskDialog.h"
#include "editor/UI/TaskPanel/ShapeHelper.h"
namespace MOON {
	enum RevolutionType
	{
		ReAdditive,
		ReSubtractive
	};
	class RevolutionTask : public ParamTaskDialog, public ShapeHelper
	{
	public:
		explicit RevolutionTask(RevolutionType type ,QWidget* parent = nullptr);
		~RevolutionTask();
		virtual QVariant getParamValue(const QString& propertyName);
		virtual void setParamValue(const QString& propertyName, const QVariant& value);
		virtual void clickOk() override;
		virtual void clickApply() override;
		virtual void clickCancel() override;
		virtual bool generateShape()override;
		void onSelectAny();
	private:
		bool initilized = false;
		class Internal;
		Internal* mInternal = nullptr;
	};
}