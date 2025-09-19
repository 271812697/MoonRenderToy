#pragma once
#include "treeViewpanel.h"
#include "editor/UI/TreeViewPanel/EntityTreeModel.h"
#include "OvCore/Global/ServiceLocator.h"
#include "OvCore/SceneSystem/SceneManager.h"
#include "renderer/Context.h"
#include "pathtrace/PathTrace.h"
#include "pathtrace/Scene.h"
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
			auto& actors = OvCore::Global::ServiceLocator::Get<OvEditor::Core::Context>().sceneManager.GetCurrentScene()->GetActors();
			root.val = "root";
			root.childs.push_back(TreeNode("scene", &root));
			for (auto& item : actors) {
				root.childs[0].childs.push_back(TreeNode(item->GetName(), &root.childs[0]));
			}
			root.childs.push_back(TreeNode("pathtrace", &root));
			if (!PathTrace::GetScene()) {
				return;
			}
			auto& meshR = PathTrace::GetScene()->meshInstancesRoots;
			auto& meshT = PathTrace::GetScene()->meshInstancesTree;
			auto& meshI = PathTrace::GetScene()->meshInstances;
			for (auto& r : meshR) {
				std::vector<int>stack0 = { r };
				std::vector<TreeNode*>stack1 = { &root.childs[1] };
				while (!stack0.empty())
				{
					int curI = stack0.back(); stack0.pop_back();
					TreeNode* curP = stack1.back(); stack1.pop_back();
					curP->childs.push_back(TreeNode(meshI[curI].name, curP));
					TreeNode* nextP = &curP->childs.back();
					for (int i = 0; i < meshT[curI].size(); i++) {
						stack0.push_back(meshT[curI][i]);
						stack1.push_back(nextP);
					}
				}
			}
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
	TreeViewModel* treemodel = nullptr;
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

	
	class TreeViewPanel::TreeViewPanelInternal {
	public:
		TreeViewPanelInternal(TreeViewPanel* tree):mSelf(tree) {
			mModel = new EntityTreeModel(mSelf);
		}
		~TreeViewPanelInternal() {

		}
	private:
		friend TreeViewPanel;
		EntityTreeModel* mModel = nullptr;
		TreeViewPanel* mSelf = nullptr;
	};
	TreeViewPanel::TreeViewPanel(QWidget* parent) :QTreeView(parent),mInternal(new TreeViewPanelInternal(this))
	{
		COPROVITE(TreeViewPanel, *this);
		QSizePolicy sizePolicy8(QSizePolicy::Preferred, QSizePolicy::Expanding);
		sizePolicy8.setHorizontalStretch(0);
		sizePolicy8.setVerticalStretch(0);
		sizePolicy8.setWidthForHeight(true);
		sizePolicy8.setHeightForWidth(this->sizePolicy().hasHeightForWidth());
		this->setSizePolicy(sizePolicy8);
		this->setModel(mInternal->mModel);


	}
	TreeViewPanel::~TreeViewPanel()
	{
		delete mInternal;
	}

}