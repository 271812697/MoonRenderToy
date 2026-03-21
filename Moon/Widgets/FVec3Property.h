#pragma once
#include "Widgets/Property.h"
#include "Widgets/FVec3.h"
namespace MOON {
	class FVec3Property :public WidgetProperty {
	public:
		FVec3Property(const QString& n, PropertyComponent* comp);
		~FVec3Property();
		virtual PropertyQtWidget* createEditorWidget(QWidget* parent = nullptr)override;
	};
}