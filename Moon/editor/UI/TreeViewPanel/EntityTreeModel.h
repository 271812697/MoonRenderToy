#pragma once
#include <vector>
#include <string>
#include <QTreeView>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

namespace MOON
{

	class EntityTreeModel : public QStandardItemModel
	{
		Q_OBJECT
			
	public:
		// 构造函数
		EntityTreeModel(QObject* parent);
	private:
		QTreeView* m_treeView = nullptr;// treeView
		QModelIndex	m_currentSelect;   
	};
}
