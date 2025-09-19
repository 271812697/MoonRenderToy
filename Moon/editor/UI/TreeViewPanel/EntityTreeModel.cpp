#include "EntityTreeModel.h"
namespace MOON {
	EntityTreeModel::EntityTreeModel(QObject* parent):
		QStandardItemModel(parent)
	{
		QStandardItem* rootItem = new QStandardItem;
		rootItem->setText(QString("SceneActor"));
		//rootItem->setIcon(m_iconMaps["root"]);
		invisibleRootItem()->setChild(invisibleRootItem()->rowCount(), 0, rootItem);
		rootItem = new QStandardItem;
		rootItem->setText(QString("PathTrace"));
		invisibleRootItem()->setChild(invisibleRootItem()->rowCount(), 0, rootItem);
	}
}