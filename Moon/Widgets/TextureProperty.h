#pragma once
#include "Widgets/Property.h"

namespace MOON {
	class TextureProperty :public WidgetProperty {
	public:
		TextureProperty(const QString& n, PropertyComponent* comp);
		~TextureProperty();
		virtual PropertyQtWidget* createEditorWidget(QWidget* parent = nullptr)override;
	};

}