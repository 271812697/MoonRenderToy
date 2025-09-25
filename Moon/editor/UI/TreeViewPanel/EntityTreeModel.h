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
		QStandardItem* sceneRoot();
		QStandardItem* pathRoot();
	public slots:
		//void OnExpandedFilter(const QModelIndex& pIndex);
		void onCheckStageChange(QStandardItem* item);
	
	private:
		class EntityTreeModelInternal;
		EntityTreeModelInternal* mInternl = nullptr;

	};
}
