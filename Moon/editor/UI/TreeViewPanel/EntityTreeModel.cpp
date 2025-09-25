#include "EntityTreeModel.h"
#include "OvCore/Global/ServiceLocator.h"
#include "OvCore/SceneSystem/Scene.h"
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
		COPROVITE(EntityTreeModel, *this);
		connect(this ,&QStandardItemModel::itemChanged,this,&EntityTreeModel::onCheckStageChange);
	}
	EntityTreeModel::~EntityTreeModel()
	{
		delete mInternl;
	}
	void EntityTreeModel::onSceneRootChange()
	{
		OvCore::SceneSystem::Scene* scene = OVSERVICE(OvEditor::Core::Context).sceneManager.GetCurrentScene();
		if (scene == nullptr) {
			return;
		}
		mInternl->sceneRoot->removeRows(0, mInternl->sceneRoot->rowCount());
		auto& actors = scene->GetActors();
		for (int i = 0; i < actors.size(); i++) {
			if (!actors[i]->HasParent()) {
				std::vector < QStandardItem*> root = { mInternl->sceneRoot };
				std::vector<OvCore::ECS::Actor*> s = { actors[i] };
				while (!s.empty()) {
					OvCore::ECS::Actor* cur = s.back(); s.pop_back();
					QStandardItem* parent = root.back(); root.pop_back();
					QStandardItem* temp = new QStandardItem;
					temp->setText(QString::fromStdString(cur->GetName()));
					//temp->setIcon(mInternl->mIconMaps["eyeOpen"]);
					temp->setCheckable(true);
					temp->setCheckState(Qt::CheckState::Checked);
					temp->setData(QVariant::fromValue((void*)cur),Qt::UserRole);
					parent->setChild(parent->rowCount(), temp);
					
					for (auto& child : cur->GetChildren()) {
						s.push_back(child);
						root.push_back(temp);
					}
				}
			}
		}

	}
	QStandardItem* EntityTreeModel::sceneRoot()
	{
		return mInternl->sceneRoot;
	}
	QStandardItem* EntityTreeModel::pathRoot()
	{
		return mInternl->pathRoot;
	}
	void EntityTreeModel::onCheckStageChange(QStandardItem* item)
	{
		if (item->isCheckable()) {
			OvCore::ECS::Actor* actor=static_cast<OvCore::ECS::Actor*>(item->data(Qt::UserRole).value<void*>());
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