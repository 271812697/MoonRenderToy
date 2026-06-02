#pragma once
#include <QStandardItemModel>
namespace Core::ECS {
	class Actor;
}
namespace MOON
{
	class TreeViewPanel;
	class EntityTreeModel : public QStandardItemModel
	{
		Q_OBJECT
	public:
		EntityTreeModel(TreeViewPanel* parent);
		~EntityTreeModel();
		void onSceneRootChange();
		void onPathRootChange();
		QStandardItem* sceneRoot();
		QStandardItem* pathRoot();
		QStandardItem* actorItem(Core::ECS::Actor* actor);
		
	public slots:
		//void OnExpandedFilter(const QModelIndex& pIndex);
		void onCheckStageChange(QStandardItem* item);
	private:
		class EntityTreeModelInternal;
		EntityTreeModelInternal* mInternl = nullptr;
	};
}
