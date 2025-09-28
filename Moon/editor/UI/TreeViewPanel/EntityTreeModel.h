#pragma once
#include <QStandardItemModel>

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

	public slots:
		//void OnExpandedFilter(const QModelIndex& pIndex);
		void onCheckStageChange(QStandardItem* item);
		
	private:

	private:
		class EntityTreeModelInternal;
		EntityTreeModelInternal* mInternl = nullptr;

	};
}
