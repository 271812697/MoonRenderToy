#pragma once
#include "treeViewpanel.h"
#include <QFileSystemModel>
#include <QAbstractItemModel>
#include <string>
#include <vector>

namespace MOON {
	struct TreeNode {
		std::string val;
		std::vector<TreeNode>childs;
		TreeNode* parent = nullptr;
		TreeNode(const std::string& v, TreeNode* p = nullptr) :val(v), parent(p) {}
		TreeNode() = default;
	};
	class TreeViewModel : public QAbstractItemModel {
	public:
		TreeViewModel(QObject* parent) :QAbstractItemModel(parent) {
			root.val = "root";
			root.childs.push_back(TreeNode("a1", &root));
			root.childs.push_back(TreeNode("a2", &root));
			root.childs.push_back(TreeNode("a3", &root));
			root.childs.push_back(TreeNode("a4", &root));
			root.childs.push_back(TreeNode("a5", &root));
			root.childs[0].childs.push_back(TreeNode("b1", &root.childs[0]));
			root.childs[0].childs.push_back(TreeNode("b2", &root.childs[0]));
			root.childs[0].childs.push_back(TreeNode("b3", &root.childs[0]));
			root.childs[0].childs.push_back(TreeNode("b4", &root.childs[0]));
			root.childs[0].childs[0].childs.push_back(TreeNode("c1", &root.childs[0].childs[0]));
			root.childs[0].childs[0].childs.push_back(TreeNode("c2", &root.childs[0].childs[0]));
			root.childs[0].childs[0].childs.push_back(TreeNode("c3", &root.childs[0].childs[0]));
			root.childs[0].childs[0].childs.push_back(TreeNode("c4", &root.childs[0].childs[0]));

		}

		int columnCount(const QModelIndex&) const override;
		bool setData(const QModelIndex& idx, const QVariant& value, int role) override;
		QVariant data(const QModelIndex& idx, int role) const override;

		QModelIndex index(int row, int column, const QModelIndex&) const override;

		QModelIndex parent(const QModelIndex&) const override;

		int rowCount(const QModelIndex&) const override;

		QVariant headerData(int section, Qt::Orientation, int role) const override;
	private:
		TreeNode root;

	};

	int TreeViewModel::columnCount(const QModelIndex&) const
	{
		return 1;
	}
	bool TreeViewModel::setData(const QModelIndex& idx, const QVariant& value, int role)
	{
		if (role != Qt::DisplayRole && role != Qt::EditRole)
		{
			return false;
		}
		return true;
	}
	QVariant TreeViewModel::data(const QModelIndex& idx, int role) const
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			if (!idx.isValid()) {
				return QString::fromStdString(root.val);
			}
			TreeNode* ptr = reinterpret_cast<TreeNode*>(idx.internalPointer());

			TreeNode* fi = &ptr->childs[idx.row()];
			return QString::fromStdString(fi->val);
		}
		return QVariant();
	}
	QModelIndex TreeViewModel::index(int row, int column, const QModelIndex& p) const
	{
		if (!p.isValid())
		{
			TreeNode* fi = const_cast<TreeNode*>(&root);
			return this->createIndex(row, column, fi);
		}
		TreeNode* ptr = reinterpret_cast<TreeNode*>(p.internalPointer());

		TreeNode* fi = &ptr->childs[p.row()];
		return this->createIndex(row, column, fi);
	}
	QModelIndex TreeViewModel::parent(const QModelIndex& idx) const
	{
		if (!idx.isValid())
		{
			return QModelIndex();
		}

		const TreeNode* ptr = reinterpret_cast<TreeNode*>(idx.internalPointer());
		if (ptr->parent == nullptr) {
			return QModelIndex();
		}
		for (int i = 0; i < ptr->parent->childs.size(); i++) {
			if (&ptr->parent->childs[i] == ptr) {
				return this->createIndex(i, 0, ptr->parent);
			}
		}
		return QModelIndex();
	}
	int TreeViewModel::rowCount(const QModelIndex& idx) const
	{
		if (!idx.isValid())
		{
			return root.childs.size();
		}
		const TreeNode* ptr =
			reinterpret_cast<TreeNode*>(idx.internalPointer());
		if (ptr != nullptr && idx.row() >= 0 &&
			idx.row() < ptr->childs.size())
		{
			return ptr->childs[idx.row()].childs.size();
		}
		return 0;
	}
	QVariant TreeViewModel::headerData(int section, Qt::Orientation, int role) const
	{
		switch (role)
		{
		case Qt::DisplayRole:
			switch (section)
			{
			case 0:
				return tr("Entity");
				break;
			case 1:
				return tr("Type");
				break;
			case 2:
				return tr("Size");
				break;
			case 3:
				return tr("Date Modified");
				break;
			}
		}

		return QVariant();
	}
	TreeViewPanel::TreeViewPanel(QWidget* parent) :QTreeView(parent)
	{
		static QFileSystemModel model;
		model.setRootPath("");

		TreeViewModel* treemodel = new TreeViewModel(this);
		//setModel(&model);
		setModel(treemodel);
		//model.setOption(QFileSystemModel::DontUseCustomDirectoryIcons);
		//const QModelIndex rootIndex = model.index(QDir::cleanPath("."));
		//if (rootIndex.isValid())
			//setRootIndex(rootIndex);
	}
}