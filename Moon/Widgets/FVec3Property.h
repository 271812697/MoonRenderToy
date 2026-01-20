#pragma once
#include "Widgets/Property.h"
#include "Widgets/FVec3.h"
namespace MOON {
	class FVec3Property :public Property {
	public:
		FVec3Property(const QString& n, PropertyComponent* comp);
		~FVec3Property();
		virtual PropertyQtWidget* createEditorWidget(QWidget* parent = nullptr)override;
		virtual void setOwnerPropertyValue(const QVariant& value)override;
		virtual void onWidgetValueChange()override;
		virtual void updateWidgetValue(const QVariant& value)override;
	private:
		Fvec3* widget = nullptr;
	};

}