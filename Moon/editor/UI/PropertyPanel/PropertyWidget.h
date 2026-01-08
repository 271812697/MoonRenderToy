#pragma once
#include <QWidget>
namespace Core::ECS {
	class Actor;
}
namespace MOON {
	class PropertyWidget : public QWidget
	{
	public:
		PropertyWidget(QWidget* parent);
		~PropertyWidget();
		void setSelectedActor(Core::ECS::Actor* actor);
	private:
		class PropertyWidgetInternal;
		PropertyWidgetInternal* mInternal=nullptr;
	};
}