#pragma once
#include "Widgets/Property.h"
#include "Widgets/ComboBox.h"
namespace MOON {
	class EnumProperty :public WidgetProperty {
	public:
		EnumProperty(const QString& n, PropertyComponent* comp);
		~EnumProperty();
		virtual PropertyQtWidget* createEditorWidget(QWidget* parent = nullptr)override;
	};

}