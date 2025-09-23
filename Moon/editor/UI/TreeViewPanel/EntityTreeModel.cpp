#include "EntityTreeModel.h"
#include "OvCore/Global/ServiceLocator.h"
#include "OvCore/SceneSystem/Scene.h"
#include "renderer/Context.h"

namespace MOON {
	class EntityTreeModel::EntityTreeModelInternal {
	public:
		EntityTreeModelInternal(EntityTreeModel* model, QTreeView* tree) :self(model), mTreeView(tree) {

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
		}
	private:
		friend EntityTreeModel;
		QTreeView* mTreeView = nullptr;// treeView
		QModelIndex	mCurrentSelect;
		EntityTreeModel* self = nullptr;
		QStandardItem* sceneRoot = nullptr;
		QStandardItem* pathRoot = nullptr;
	};
	EntityTreeModel::EntityTreeModel(QTreeView* parent) :
		QStandardItemModel(parent), mInternl(new EntityTreeModelInternal(this, parent))
	{
		mInternl->init();
		COPROVITE(EntityTreeModel, *this);

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
}