#include "Widgets/TextureProperty.h"
#include "Widgets/PropertyComponent.h"
#include "Widgets/TextureDrop.h"
namespace MOON {
	TextureProperty::TextureProperty(const QString& n, PropertyComponent* comp) :WidgetProperty(n, comp) {

	}
	TextureProperty::~TextureProperty() {

	}
	PropertyQtWidget* TextureProperty::createEditorWidget(QWidget* parent) {
		if (mWidget == nullptr) {
			auto temp= new TextureDrop(parent, this);
			mWidget = temp;
			//temp->addComboList(owner->getPropertyValue(mName).value<QList<QString>>());
			
		}
		return mWidget;
	}
}