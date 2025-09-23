#pragma once
#include <vector>
#include <string>
#include <QTreeView>
#include <QStandardItemModel>

namespace MOON
{

	class EntityTreeModel : public QStandardItemModel
	{
		Q_OBJECT

	public:
		// 构造函数
		EntityTreeModel(QTreeView* parent);
		~EntityTreeModel();
		void onSceneRootChange();
		QStandardItem* sceneRoot();
		QStandardItem* pathRoot();
	private:
		class EntityTreeModelInternal;
		EntityTreeModelInternal* mInternl = nullptr;

	};
}
