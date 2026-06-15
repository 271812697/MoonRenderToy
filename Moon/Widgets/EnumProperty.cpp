#include "Widgets/EnumProperty.h"
#include "Widgets/PropertyComponent.h"
namespace MOON {
	EnumProperty::EnumProperty(const QString& n, PropertyComponent* comp) :WidgetProperty(n, comp) {

	}
	EnumProperty::~EnumProperty() {

	}
	PropertyQtWidget* EnumProperty::createEditorWidget(QWidget* parent) {
		if (mWidget == nullptr) {
			auto temp= new ComboBox(parent, this);
			mWidget = temp;
			temp->addComboList(owner->getPropertyValue(mName).value<QList<QString>>());	
		}
		return mWidget;
	}
}