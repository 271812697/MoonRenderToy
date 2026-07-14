#pragma once
#include "editor/UI/TaskPanel/ParamTaskDialog.h"
#include "editor/UI/TaskPanel/ShapeHelper.h"
namespace MOON {
	enum ExtrudeType
	{
		Additive,
		Subtractive
	};
	class ExtrudeTaskDialog : public ParamTaskDialog,public ShapeHelper
	{
		Q_OBJECT
	public:
		explicit ExtrudeTaskDialog(QWidget* parent = nullptr, ExtrudeType type=Additive);
		virtual ~ExtrudeTaskDialog()override;
		
		virtual QVariant getParamValue(const QString& propertyName)override;
		virtual void setParamValue(const QString& propertyName, const QVariant& value)override;
		virtual void clickOk() override;
		virtual void clickApply() override;
		virtual void clickCancel() override;
		virtual bool generateShape()override;
		void onValueChange();
		void onAngleChange();
		void onLengthChange();
		void setUp();
	private:
		virtual void onSelectFace(const std::vector<Part::TopoShape>& face)override;
	private:
		void onWidgetLengthInvoke();
		void onWidgetAngleInvoke();
		class Internal;
		Internal* mInternal = nullptr;
	};
}