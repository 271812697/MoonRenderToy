#pragma once
#include "Widgets/Property.h"
#include "Widgets/FVec4.h"
namespace MOON {
	class FVec4Property :public WidgetProperty {
	public:
		FVec4Property(const QString& n, PropertyComponent* comp);
		~FVec4Property();
		virtual PropertyQtWidget* createEditorWidget(QWidget* parent = nullptr)override;
	};
}