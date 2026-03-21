#include "Widgets/FVec4Property.h"
#include "Widgets/PropertyComponent.h"
namespace MOON {
	FVec4Property::FVec4Property(const QString& n, PropertyComponent* comp) :WidgetProperty(n, comp) {

	}
	FVec4Property::~FVec4Property() {

	}
	PropertyQtWidget* FVec4Property::createEditorWidget(QWidget* parent ){
		if (mWidget == nullptr) {
			mWidget = new Fvec4(parent, this);
			updateWidgetValue(owner->getPropertyValue(mName));
		}
		return mWidget;
	}
}