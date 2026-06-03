#pragma once
#include <QtWidgets/QTreeView>
#include <QModelIndex>
#include <QStandardItem>
namespace Core::ECS {
	class Actor;
}
namespace MOON {
	class TreeViewPanel;

	class TreeViewPanel : public QTreeView
	{
		Q_OBJECT
	public:
		TreeViewPanel(QWidget* parent);
		~TreeViewPanel();
		void updateTreeViewSketcherRoot();
	signals:
		void setSelectActor(Core::ECS::Actor* actor);
		void itemHovered(Core::ECS::Actor* actor);   // 悬浮
		void itemLeave(Core::ECS::Actor* actor);
	public slots:
		void updateTreeViewSceneRoot();
		// 外部调用：根据 Actor 指针高亮 TreeView 项
		void highlightByActor(Core::ECS::Actor* actor);
		// 清空高亮
		void clearHighlight();	
	public:
		QModelIndex m_highlightIndex; // 用来保存当前高亮index

	protected:
		
		void mousePressEvent(QMouseEvent* event) override;
		void mouseMoveEvent(QMouseEvent* event) override;

	private:
		class TreeViewPanelInternal;
		TreeViewPanelInternal* mInternal;
	};
}