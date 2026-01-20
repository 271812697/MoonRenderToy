#pragma once
#include "Widgets/Property.h"
#include "Widgets/ComboBox.h"
namespace MOON {
	class EnumProperty :public Property {
	public:
		EnumProperty(const QString& n, PropertyComponent* comp);
		~EnumProperty();
		virtual PropertyQtWidget* createEditorWidget(QWidget* parent = nullptr)override;
		virtual void setOwnerPropertyValue(const QVariant& value)override;
		virtual void onWidgetValueChange()override;
		virtual void updateWidgetValue(const QVariant& value)override;
	private:
		ComboBox* widget = nullptr;
	};

}