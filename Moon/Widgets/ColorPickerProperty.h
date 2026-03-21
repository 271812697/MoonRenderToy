#pragma once
#include "Widgets/Property.h"
#include "Widgets/ColorPicker.h"
namespace MOON {
	class ColorPickerProperty :public WidgetProperty {
	public:
		ColorPickerProperty(const QString& n, PropertyComponent* comp);
		~ColorPickerProperty();
		virtual PropertyQtWidget* createEditorWidget(QWidget* parent = nullptr)override;
	};

}