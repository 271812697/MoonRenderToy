#pragma once
#include "editor/UI/TaskPanel/BaseTaskDialog.h"
#include <unordered_map>
#include <QVariant>
namespace MOON {
	class CollapsibleGroupBoxWidget;
	class PropertyComponent;
	class WidgetProperty;
	class ParamTaskDialog : public BaseTaskDialog
	{
		Q_OBJECT
	public:
		explicit ParamTaskDialog(QWidget* parent = nullptr);
		~ParamTaskDialog();
		void addParam(WidgetProperty* param);
		PropertyComponent* addGroupParam(const QString& groupName);
		PropertyComponent* getGroupComponent(const QString& groupName);
		virtual void buildUi() override final;
		virtual void clickOk() override;
		virtual void clickApply() override;
		virtual void clickCancel() override;
		virtual QVariant getParamValue(const QString& propertyName);
		virtual void setParamValue(const QString& propertyName, const QVariant& value);
	protected:	
		std::vector<std::pair<CollapsibleGroupBoxWidget*, PropertyComponent*>>m_comps;
		std::unordered_map<QString, int>groupToIndex;
	};
}