#pragma once
#include "core/ECS/Actor.h"
#include "editor/UI/PropertyPanel/PropertyModel.h"
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>

namespace MOON {
	PropertyTreeModel::PropertyTreeModel(QObject* parent) : QAbstractItemModel(parent)
	{
		m_rootNode = new NodeData{ ActorType, QVariant(), 0, nullptr };
	}
	QModelIndex PropertyTreeModel::index(int row, int column, const QModelIndex& parent) const
	{
        if (!hasIndex(row, column, parent))
            return QModelIndex();

        NodeData* parentNode = nodeFromIndex(parent);
        if (!parentNode)
            parentNode = m_rootNode;

        if (row < parentNode->children.size()) {
            NodeData* childNode = parentNode->children[row];
            return createIndex(row, column, childNode);
        }

        return QModelIndex();
	}
    QModelIndex PropertyTreeModel::parent(const QModelIndex& child) const
    {
        if (!child.isValid())
            return QModelIndex();

        NodeData* childNode = static_cast<NodeData*>(child.internalPointer());
        NodeData* parentNode = childNode->parent;

        if (parentNode == m_rootNode || !parentNode)
            return QModelIndex();

        return createIndex(parentNode->row, 0, parentNode);
    }
    int PropertyTreeModel::rowCount(const QModelIndex& parent) const
    {
        NodeData* parentNode = nodeFromIndex(parent);
        if (!parentNode)
            parentNode = m_rootNode;

        return parentNode->children.size();
    }
    int PropertyTreeModel::columnCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent);
        NodeData* node = static_cast<NodeData*>(parent.internalPointer());
        if (node) {
            if (node->type == ActorType|| node->type == ComponentType) {
                return 1;
            }
        }
        else
        {
			return 1; // 根节点只有一列
        }

        return 2; // 两列：属性名、属性值
    }
    QVariant PropertyTreeModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        NodeData* node = static_cast<NodeData*>(index.internalPointer());
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            switch (node->type) {
            case ActorType: {
                if (index.column() == 0)
                    return QString::fromStdString(m_currentActor->GetName());
                break;
            }
            case ComponentType: {
                Component comp = node->data.value<Component>();
                if (index.column() == 0)
                    return comp.name;
                break;
            }
            case PropertyType: {
                ComponentProperty prop = node->data.value<ComponentProperty>();
                if (index.column() == 0)
                    return prop.name;
                else if (index.column() == 1)
                    return prop.value;
                break;
            }
            }
        }

        return QVariant();
    }
    bool PropertyTreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
    {
        if (role != Qt::EditRole || !index.isValid())
            return false;

        NodeData* node = static_cast<NodeData*>(index.internalPointer());
        if (node->type == PropertyType && index.column() == 1) {
            // 更新属性值
            ComponentProperty prop = node->data.value<ComponentProperty>();
            prop.value = value;
            node->data = QVariant::fromValue(prop);

            // 更新原始Actor数据（同步）
            int compRow = node->parent->row;
            int propRow = node->row;
            //m_currentActor.components[compRow].properties[propRow].value = value;

            emit dataChanged(index, index);
            return true;
        }

        return false;
    }
    Qt::ItemFlags PropertyTreeModel::flags(const QModelIndex& index) const
    {
        if (!index.isValid())
            return Qt::NoItemFlags;

        Qt::ItemFlags flags = QAbstractItemModel::flags(index);
        NodeData* node = static_cast<NodeData*>(index.internalPointer());

        // 只有属性值列可编辑
        if (node->type == PropertyType && index.column() == 1) {
            flags |= Qt::ItemIsEditable;
        }

        return flags;
    }
    void PropertyTreeModel::setCurrentActor(Core::ECS::Actor* actor)
    {
        beginResetModel();
        m_currentActor = actor;
        buildNodeTree();
        endResetModel();
    }
    PropertyTreeModel::NodeData* PropertyTreeModel::nodeFromIndex(const QModelIndex& index) const
    {
        if (index.isValid()) {
            return static_cast<NodeData*>(index.internalPointer());
        }
        return nullptr;
    }
    void PropertyTreeModel::buildNodeTree()
    {
        // 清空原有节点
        qDeleteAll(m_rootNode->children);
        m_rootNode->children.clear();

        // 1. 创建Actor节点（根节点的子节点）
        NodeData* actorNode = new NodeData{ ActorType, QVariant(), 0, m_rootNode };
        m_rootNode->children.append(actorNode);

        // 2. 为每个组件创建节点
        int i = 0;
        for (auto& ptr:m_currentActor->GetComponents()) {
            Component comp;
            comp.name=QString::fromStdString(ptr->GetName());
            NodeData* compNode = new NodeData{ ComponentType, QVariant::fromValue(comp), i++, actorNode };
            actorNode->children.append(compNode);
            // 3. 为每个组件的属性创建节点
            for (int j = 0; j < comp.properties.size(); ++j) {
                const ComponentProperty& prop = comp.properties[j];
                NodeData* propNode = new NodeData{ PropertyType, QVariant::fromValue(prop), j, compNode };
                compNode->children.append(propNode);
            }
        }
    }

    PropertyDelegate::PropertyDelegate(QObject* parent) : QItemDelegate(parent)
    {

    }

    QWidget* PropertyDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        PropertyTreeModel* model = qobject_cast<PropertyTreeModel*>(const_cast<QAbstractItemModel*>(index.model()));
        if (!model)
            return QItemDelegate::createEditor(parent, option, index);

        PropertyTreeModel::NodeData* node = model->nodeFromIndex(index);
        if (node->type != PropertyTreeModel::PropertyType)
            return QItemDelegate::createEditor(parent, option, index);

        // 根据属性类型创建不同编辑器
        ComponentProperty prop = node->data.value<ComponentProperty>();
        switch (prop.type) {
        case QVariant::Int: {
            QSpinBox* spinBox = new QSpinBox(parent);
            spinBox->setRange(-9999, 9999);
            return spinBox;
        }
        case QVariant::Bool: {
            QCheckBox* checkBox = new QCheckBox(parent);
            return checkBox;
        }
        default: { // 字符串等其他类型
            QLineEdit* lineEdit = new QLineEdit(parent);
            return lineEdit;
        }
        }
    }

    void PropertyDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
    {
        QVariant value = index.model()->data(index, Qt::EditRole);

        if (QSpinBox* spinBox = qobject_cast<QSpinBox*>(editor)) {
            spinBox->setValue(value.toInt());
        }
        else if (QCheckBox* checkBox = qobject_cast<QCheckBox*>(editor)) {
            checkBox->setChecked(value.toBool());
        }
        else if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editor)) {
            lineEdit->setText(value.toString());
        }
        else {
            QItemDelegate::setEditorData(editor, index);
        }
    }

    void PropertyDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
    {
		if (QSpinBox* spinBox = qobject_cast<QSpinBox*>(editor)) {
			model->setData(index, spinBox->value(), Qt::EditRole);
		}
		else if (QCheckBox* checkBox = qobject_cast<QCheckBox*>(editor)) {
			model->setData(index, checkBox->isChecked(), Qt::EditRole);
		}
		else if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editor)) {
			model->setData(index, lineEdit->text(), Qt::EditRole);
		}
		else {
			QItemDelegate::setModelData(editor, model, index);
		}
    }

}