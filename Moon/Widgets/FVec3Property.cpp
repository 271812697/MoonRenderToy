#include "Widgets/FVec3Property.h"
#include "Widgets/PropertyComponent.h"
namespace MOON {
	FVec3Property::FVec3Property(const QString& n, PropertyComponent* comp) :WidgetProperty(n, comp) {

	}
	FVec3Property::~FVec3Property() {

	}
	PropertyQtWidget* FVec3Property::createEditorWidget(QWidget* parent ){
		if (mWidget == nullptr) {
			mWidget = new Fvec3(parent, this);
			updateWidgetValue(owner->getPropertyValue(mName));
		}
		return mWidget;
	}
}