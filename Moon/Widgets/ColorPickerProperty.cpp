#include "Widgets/ColorPickerProperty.h"
#include "Widgets/PropertyComponent.h"
namespace MOON {
	ColorPickerProperty::ColorPickerProperty(const QString& n, PropertyComponent* comp) :WidgetProperty(n, comp) {

	}
	ColorPickerProperty::~ColorPickerProperty() {

	}
	PropertyQtWidget* ColorPickerProperty::createEditorWidget(QWidget* parent){
		if (mWidget == nullptr) {
			mWidget = new ColorPicker(parent, this);
			updateWidgetValue(owner->getPropertyValue(mName));
		}
		return mWidget;
	}
}