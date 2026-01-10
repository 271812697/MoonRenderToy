#pragma once
#include "editor/UI/PropertyPanel/PropertyComponent.h"
#include <QWidget>
#include <QAbstractItemModel>
#include <QItemDelegate>
#include <QStyledItemDelegate>
#include <vector>
namespace Core::ECS {
    class Actor;
}
namespace MOON {
 
    // 自定义属性模型
    class PropertyTreeModel : public QAbstractItemModel
    {
        Q_OBJECT
    public:
        enum ItemType {
            ActorType,      // Actor节点
            ComponentType,  // 组件节点
            PropertyType    // 属性节点
        };

        explicit PropertyTreeModel(QObject* parent = nullptr);

        // 重写QAbstractItemModel纯虚函数
        QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex& child) const override;
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
        Qt::ItemFlags flags(const QModelIndex& index) const override;

        // 设置当前选中的Actor
        void setCurrentActor( Core::ECS::Actor* actor);
        // 节点数据结构体
        struct NodeData {
            ItemType type;
            QVariant data;
            int row;                 // 所在行
            NodeData* parent = nullptr;
            QList<NodeData*> children;

            ~NodeData() {
                qDeleteAll(children);
            }
        };
        // 根据索引获取节点
        NodeData* nodeFromIndex(const QModelIndex& index) const;
    private:


        NodeData* m_rootNode;        // 根节点
        Core::ECS::Actor* m_currentActor;        // 当前选中的Actor
        std::vector<ActorPropertyComponent*>m_comps;

        // 构建节点树
        void buildNodeTree();
    };
    // 自定义委托（为不同属性提供不同编辑器）
    class PropertyDelegate : public QStyledItemDelegate
    {
        Q_OBJECT
    public:
        explicit PropertyDelegate(QObject* parent = nullptr);

        // 创建编辑器
        QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;
        // 设置编辑器数据
        void setEditorData(QWidget* editor, const QModelIndex& index) const override;
        // 提交编辑器数据
        void setModelData(QWidget* editor, QAbstractItemModel* model,
            const QModelIndex& index) const override;
        };




}