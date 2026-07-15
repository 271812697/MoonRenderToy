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
		enum OperationType
		{
			Add,
			Remove,
			Update
		};
		struct Operation
		{
			OperationType type;
			std::vector<Core::ECS::Actor*> actors;
		};
		TreeViewPanel(QWidget* parent);
		~TreeViewPanel();
		void updateTreeViewSketcherRoot();
		void addActorToTree(const std::vector<Core::ECS::Actor*>& actor);
		void removeActorFromTree(const std::vector<Core::ECS::Actor*>& actor);
		void updateActorInTree(const std::vector<Operation>&operations);
	signals:
		void setSelectActor(Core::ECS::Actor* actor);
		void itemHovered(Core::ECS::Actor* actor);   // 悬浮
		void itemLeave(Core::ECS::Actor* actor);
		void selectFeature(void* feature);
	public slots:
		void updateTreeViewSceneRoot();
		// 外部调用：根据 Actor 指针高亮 TreeView 项
		void highlightByActor(Core::ECS::Actor* actor);
		// 清空高亮
		void clearHighlight();	
		void clearLastHoverIndex();
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