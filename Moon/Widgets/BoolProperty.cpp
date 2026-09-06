#include "Widgets/BoolProperty.h"
#include "Widgets/PropertyComponent.h"
namespace MOON {
	BoolProperty::BoolProperty(
		const QString& n,
		PropertyComponent* comp,
		Style style
	) :WidgetProperty(n, comp), m_style(style) {
	}
	BoolProperty::~BoolProperty() {

	}
	PropertyQtWidget* BoolProperty::createEditorWidget(QWidget* parent ) {
		if (mWidget == nullptr) {
			if (m_style == Style::InviwoRect) {
				mWidget = new InviwoCheckBox(parent, this);
			}
			else {
				mWidget = new SliderCheckBox(parent, this);
			}
			updateWidgetValue(owner->getPropertyValue(mName));
		}
		return mWidget;
	}

}
