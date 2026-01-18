#pragma once
#include "Widgets/Property.h"
#include "Widgets/ColorPicker.h"
namespace MOON {
	class ColorPickerProperty :public Property {
	public:
		ColorPickerProperty(const QString& n, PropertyComponent* comp);
		~ColorPickerProperty();
		virtual PropertyQtWidget* createEditorWidget(QWidget* parent = nullptr)override;
		virtual void setOwnerPropertyValue(const QVariant& value)override;
		virtual void onWidgetValueChange()override;
		virtual void updateWidgetValue(const QVariant& value)override;
	private:
		ColorPicker* widget = nullptr;
	};

}