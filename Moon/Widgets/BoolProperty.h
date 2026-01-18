#pragma once
#include "Widgets/Property.h"
#include "Widgets/SlideChekBox.h"
namespace MOON {
	class BoolProperty :public Property {
	public:
		BoolProperty(const QString& n, PropertyComponent* comp);
		~BoolProperty();
		virtual PropertyQtWidget* createEditorWidget(QWidget* parent = nullptr)override;
		virtual void setOwnerPropertyValue(const QVariant& value)override;
		virtual void onWidgetValueChange()override;
		virtual void updateWidgetValue(const QVariant& value)override;
	private:
		SliderCheckBox* widget = nullptr;
	};

}