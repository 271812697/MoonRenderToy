#pragma once
#include "Widgets/Property.h"
#include "Widgets/InviwoCheckBox.h"
#include "Widgets/SlideChekBox.h"
namespace MOON {
	class BoolProperty :public WidgetProperty {
	public:
		enum class Style {
			Slider,      // original sliding switch (default)
			InviwoRect   // Inviwo style rectangle checkbox
		};
		BoolProperty(
			const QString& n,
			PropertyComponent* comp,
			Style style = Style::Slider
		);
		~BoolProperty();
		virtual PropertyQtWidget* createEditorWidget(QWidget* parent = nullptr)override;
	private:
		Style m_style = Style::Slider;
	};

}
