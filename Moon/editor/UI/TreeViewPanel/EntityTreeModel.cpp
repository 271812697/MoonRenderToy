#include "EntityTreeModel.h"
#include "Core/Global/ServiceLocator.h"
#include "Core/SceneSystem/Scene.h"
#include "renderer/Context.h"
#include "treeViewpanel.h"


namespace MOON {

	class EntityTreeModel::EntityTreeModelInternal {
	public:
		EntityTreeModelInternal(EntityTreeModel* model, TreeViewPanel* tree) :self(model), mTreeView(tree) {

		}
		void init() {
			QList<QStandardItem*> arrays;
			sceneRoot = new QStandardItem;
			sceneRoot->setText(QString("SceneActor"));
			arrays.push_back(sceneRoot);
			pathRoot = new QStandardItem;
			pathRoot->setText(QString("PathTrace"));

			arrays.push_back(pathRoot);
			self->invisibleRootItem()->appendColumn(arrays);
			mIconMaps["eyeOpen"] = QIcon(":/entityTree/icons/pqEyeball.svg");
			mIconMaps["eyeClose"] = QIcon(":/entityTree/icons/pqEyeballClosed.svg");

		}
	private:
		friend EntityTreeModel;
		TreeViewPanel* mTreeView = nullptr;// treeView
		QModelIndex	mCurrentSelect;
		EntityTreeModel* self = nullptr;
		QStandardItem* sceneRoot = nullptr;
		QStandardItem* pathRoot = nullptr;
		std::unordered_map<std::string, QIcon>mIconMaps;

	};
	EntityTreeModel::EntityTreeModel(TreeViewPanel* parent) :
		QStandardItemModel(parent), mInternl(new EntityTreeModelInternal(this, parent))
	{
		mInternl->init();
		RegService(EntityTreeModel, *this);
		connect(this, &QStandardItemModel::itemChanged, this, &EntityTreeModel::onCheckStageChange);
	}
	EntityTreeModel::~EntityTreeModel()
	{
		delete mInternl;
	}
	void EntityTreeModel::onSceneRootChange()
	{
		Core::SceneSystem::Scene* scene = GetService(Editor::Core::Context).sceneManager.GetCurrentScene();
		if (scene == nullptr) {
			return;
		}
		mInternl->sceneRoot->removeRows(0, mInternl->sceneRoot->rowCount());
		auto& actors = scene->GetActors();
		for (int i = 0; i < actors.size(); i++) {
			if (!actors[i]->HasParent()) {
				std::vector <QStandardItem*> root = { mInternl->sceneRoot };
				std::vector<Core::ECS::Actor*> s = { actors[i] };
				while (!s.empty()) {
					Core::ECS::Actor* cur = s.back(); s.pop_back();
					QStandardItem* parent = root.back(); root.pop_back();
					QStandardItem* temp = new QStandardItem;
					auto name = cur->GetName();
					temp->setText(QString::fromStdString(name));
					
					//temp->setIcon(mInternl->mIconMaps["eyeOpen"]);
					temp->setCheckable(true);
					temp->setCheckState(Qt::Checked);
					temp->setData(QVariant::fromValue((void*)cur), Qt::UserRole);
					parent->setChild(parent->rowCount(), temp);

					for (auto& child : cur->GetChildren()) {
						s.push_back(child);
						root.push_back(temp);
					}
				}
			}
		}

	}
	void EntityTreeModel::onPathRootChange()
	{
		
		
	}
	QStandardItem* EntityTreeModel::sceneRoot()
	{
		return mInternl->sceneRoot;
	}
	QStandardItem* EntityTreeModel::pathRoot()
	{
		return mInternl->pathRoot;
	}
	// 同步子节点状态（父节点勾选/取消时调用）
	void syncChildItems(QStandardItem* parent, Qt::CheckState state) {
		for (int i = 0; i < parent->rowCount(); ++i) {
			QStandardItem* child = parent->child(i);
			if (child && child->isCheckable() && child->checkState() != state) {
				child->setCheckState(state);
				// 如果子节点还有子节点，递归同步
				if (child->hasChildren()) {
					syncChildItems(child, state);
				}
			}
		}
	}

	// 同步父节点状态（子节点勾选/取消时调用）
	void syncParentItem(QStandardItem* parent) {
		int checkedCount = 0;
		int totalCount = parent->rowCount();

		// 统计已勾选的子节点数量
		for (int i = 0; i < totalCount; ++i) {
			QStandardItem* child = parent->child(i);
			if (child && child->isCheckable() && child->checkState() == Qt::Checked) {
				checkedCount++;
			}
		}

		// 根据子节点勾选情况更新父节点状态
		Qt::CheckState newState;
		if (checkedCount == 0) {
			newState = Qt::Unchecked;      // 全未勾选
		}
		else if (checkedCount == totalCount) {
			newState = Qt::Checked;        // 全勾选
		}
		else {
			newState = Qt::PartiallyChecked; // 部分勾选（半选状态）
		}

		if (parent->checkState() != newState) {
			parent->setCheckState(newState);
			// 如果父节点还有父节点，递归检查
			if (parent->parent()) {
				syncParentItem(parent->parent());
			}
		}
	}

	void EntityTreeModel::onCheckStageChange(QStandardItem* item)
	{
		//// 防止递归调用导致的死循环
		static bool isProcessing = false;
		if (isProcessing) {
			return;
		}
		isProcessing = true;

		// 如果是父节点，同步所有子节点状态
		if (item->hasChildren()) {
			syncChildItems(item, item->checkState());
		}
		// 如果是子节点，检查是否需要同步父节点状态
		if (item->parent()) {
			syncParentItem(item->parent());
		}

		isProcessing = false;
		if (item->isCheckable()) {
			Core::ECS::Actor* actor = static_cast<Core::ECS::Actor*>(item->data(Qt::UserRole).value<void*>());
			if (actor) {
				Qt::CheckState currentState = item->checkState();
				if (currentState == Qt::Checked) {
					actor->SetActive(true);
				}
				else if (currentState == Qt::Unchecked) {
					actor->SetActive(false);
				}
			}
		}
	}
}