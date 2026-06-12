#include "EntityTreeModel.h"
#include "Core/Global/ServiceLocator.h"
#include "core/component/CTopoShape.h"
#include "Core/SceneSystem/Scene.h"
#include "renderer/Context.h"
#include "treeViewpanel.h"
#include "Sketcher/SketcherObj.h"
#include "Sketcher/SketcherObjManager.h"


namespace MOON {

	class EntityTreeModel::EntityTreeModelInternal {
	public:
		EntityTreeModelInternal(EntityTreeModel* model, TreeViewPanel* tree) :self(model), mTreeView(tree) {

		}
		void init() {
			QList<QStandardItem*> arrays;
			sceneRoot = new QStandardItem;
			sceneRoot->setText(QString("SceneActor"));
			sketcherRoot = new QStandardItem;
			sketcherRoot->setText(QString("Sketcher"));
			arrays.push_back(sceneRoot);
			arrays.push_back(sketcherRoot);
			self->invisibleRootItem()->appendColumn(arrays);
			mIconMaps["eyeOpen"] = QIcon(":/entityTree/icons/pqEyeball.svg");
			mIconMaps["eyeClose"] = QIcon(":/entityTree/icons/pqEyeballClosed.svg");
			
			mIconMaps["Geomerty"] = QIcon(":/widgets/icons/Geomerty.png");
			mIconMaps["PointLight"] = QIcon(":/widgets/icons/PointLight.png");
			mIconMaps["DirectionalLight"] = QIcon(":/widgets/icons/DirectionalLight.png");
			mIconMaps["SkyBox"] = QIcon(":/widgets/icons/awesomeface.png");
			mIconMaps["PostProcessStack"] = QIcon(":/widgets/icons/awesomeface.png");
			mIconMaps[""]= QIcon(":/widgets/icons/Model.png");
			mIconMaps["Sketcher"] = QIcon(":/widgets/icons/Sketcher_NewSketch.svg");
		}
	private:
		friend EntityTreeModel;
		TreeViewPanel* mTreeView = nullptr;// treeView
		bool manaulCheck = false;
		QModelIndex	mCurrentSelect;
		EntityTreeModel* self = nullptr;
		QStandardItem* sceneRoot = nullptr;
		QStandardItem* sketcherRoot = nullptr;
		std::unordered_map<std::string, QIcon>mIconMaps;
		std::unordered_map<Core::ECS::Actor*, QStandardItem*>actorToItem;

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
	void EntityTreeModel::onSketcherChange()
	{
		//mInternl->manaulCheck = false;
		mInternl->sketcherRoot->removeRows(0, mInternl->sketcherRoot->rowCount());
		auto sketcherList=SketcherObjManager::instance().GetAllSketcherObjs();
		auto curActiveSketch=SketcherObjManager::instance().GetCurrentActiveSketcherObj();
		for (int i = 0;i < sketcherList.size();i++) {
			QStandardItem* temp = new QStandardItem;
			temp->setText(QString::fromStdString("sketcher"));
			temp->setIcon(mInternl->mIconMaps["Sketcher"]);
			mInternl->sketcherRoot->setChild(mInternl->sketcherRoot->rowCount(), temp);
			temp->setData(QVariant::fromValue((void*)sketcherList[i]), Qt::UserRole+1);
			temp->setData(QVariant::fromValue((void*)nullptr), Qt::UserRole);
			temp->setCheckable(true);
			temp->setCheckState(curActiveSketch== sketcherList[i] ? Qt::Checked : Qt::Unchecked);
		}
		//mInternl->manaulCheck =true;
	}
	void EntityTreeModel::onSceneRootChange()
	{
		Core::SceneSystem::Scene* scene = GetService(Editor::Core::Context).sceneManager.GetCurrentScene();
		if (scene == nullptr) {
			return;
		}
		mInternl->mTreeView->clearHighlight();
		mInternl->mTreeView->clearLastHoverIndex();
		mInternl->manaulCheck = false;
		mInternl->sceneRoot->removeRows(0, mInternl->sceneRoot->rowCount());
		mInternl->actorToItem.clear();
		auto& actors = scene->GetActors();
		
		for (int i = 0; i < actors.size(); i++) {
			if (!actors[i]->HasParent()) {
				std::vector <QStandardItem*> root = { mInternl->sceneRoot };
				std::vector<Core::ECS::Actor*> s = { actors[i] };
				while (!s.empty()) {
					Core::ECS::Actor* cur = s.back(); s.pop_back();
					QStandardItem* parent = root.back(); root.pop_back();
					QStandardItem* temp = new QStandardItem;
					mInternl->actorToItem[cur] = temp;
					auto name = cur->GetName();
					auto tag = cur->GetTag();
					temp->setText(QString::fromStdString(name));

					if (mInternl->mIconMaps.find(tag) != mInternl->mIconMaps.end()) {
						temp->setIcon(mInternl->mIconMaps[tag]);
					}

					//temp->setIcon(mInternl->mIconMaps["eyeOpen"]);
					temp->setCheckable(true);
					temp->setCheckState(cur->IsActive()?Qt::Checked:Qt::Unchecked);
					temp->setData(QVariant::fromValue((void*)cur), Qt::UserRole);
					parent->setChild(parent->rowCount(), temp);
	
					QModelIndex index = this->indexFromItem(temp);
					mInternl->mTreeView->expand(index);
					for (auto& child : cur->GetChildren()) {
						s.push_back(child);
						root.push_back(temp);
					}
				}
			}
		}
		mInternl->manaulCheck = true;
	}
	QStandardItem* EntityTreeModel::sceneRoot()
	{
		return mInternl->sceneRoot;
	}
	QStandardItem* EntityTreeModel::actorItem(Core::ECS::Actor* actor)
	{
		if (mInternl->actorToItem.find(actor) != mInternl->actorToItem.end()) {
			return mInternl->actorToItem[actor];
		}
		return nullptr;
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

		if (item->isCheckable()) {
			Core::ECS::Actor* actor = static_cast<Core::ECS::Actor*>(item->data(Qt::UserRole).value<void*>());
			if (actor) {
				Qt::CheckState currentState = item->checkState();
				if (currentState == Qt::Checked) {
					actor->SetActive(true);
				}
				else //if (currentState == Qt::Unchecked)
				{
					actor->SetActive(false);
				}
				if (mInternl->manaulCheck) {
					if (actor->HasComponent("CTopoShape")) {
						auto topoComp = actor->GetComponent<::Core::ECS::Components::CTopoShape>();
						topoComp->updateChildBuffer();
					}
					else if (actor->HasParent()) {
						auto parent = actor->GetParent();
						if (parent->HasComponent("CTopoShape")&&actor->GetName()=="Face") {
							auto topoComp =parent->GetComponent<::Core::ECS::Components::CTopoShape>();
							topoComp->updateChildBuffer();
						}else if (parent->HasParent()) {
							auto grandParent = parent->GetParent();
							if (grandParent->HasComponent("CTopoShape")&&parent->GetName()=="Face") {
								auto topoComp = grandParent->GetComponent<::Core::ECS::Components::CTopoShape>();
								topoComp->updateChildBuffer();
							}
						}
					}
				}
			}
			else
			{
				SketcherObj* sketcher = static_cast<SketcherObj*>(item->data(Qt::UserRole + 1).value<void*>());
				if (sketcher) {
					Qt::CheckState currentState = item->checkState();
					if (currentState == Qt::Checked) {
						sketcher->setActive(true);
					}
					else if (currentState == Qt::Unchecked) {
						sketcher->setActive(false);
					}
				}
			}
		}
		isProcessing = false;
	}
}