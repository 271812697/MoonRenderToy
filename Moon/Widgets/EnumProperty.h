#pragma once
#include "Widgets/Property.h"
#include "Widgets/ComboBox.h"
namespace MOON {
	class EnumProperty :public WidgetProperty {
	public:
		EnumProperty(const QString& n, PropertyComponent* comp);
		~EnumProperty();
		virtual PropertyQtWidget* createEditorWidget(QWidget* parent = nullptr)override;
		/** Index the combo box starts on. The owner has to supply it because
		* owner->getPropertyValue() returns the option list, not the current
		* selection, so the widget would otherwise always show the first entry
		* even when the edited object stores something else. */
		void setInitIndex(int p_index) { mInitIndex = p_index; }
	private:
		int mInitIndex = -1;
	};

}
