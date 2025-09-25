#pragma once
#include <QtWidgets/QTreeView>
namespace OvCore::ECS {
	class Actor;
}
namespace MOON {
	class TreeViewPanel : public QTreeView
	{
		Q_OBJECT
	public:
		TreeViewPanel(QWidget* parent);
		~TreeViewPanel();
	signals:
		void setSelectActor(OvCore::ECS::Actor* actor);

	public slots:
		void updateTreeViewSceneRoot();
		void updateTreeViewPathRoot();
	protected:
		
		void mousePressEvent(QMouseEvent* event) override;

	private:
		class TreeViewPanelInternal;
		TreeViewPanelInternal* mInternal;
	};
}