#include "Widgets/BoolProperty.h"
#include "Widgets/PropertyComponent.h"
namespace MOON {
	BoolProperty::BoolProperty(const QString& n, PropertyComponent* comp) :WidgetProperty(n, comp) {

	}
	BoolProperty::~BoolProperty() {

	}
	PropertyQtWidget* BoolProperty::createEditorWidget(QWidget* parent ) {
		if (mWidget == nullptr) {
			mWidget = new SliderCheckBox(parent, this);
			updateWidgetValue(owner->getPropertyValue(mName));
		}
		return mWidget;
	}

}