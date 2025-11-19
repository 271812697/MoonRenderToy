#pragma once
#include "treeViewpanel.h"
#include "editor/UI/TreeViewPanel/EntityTreeModel.h"
#include "editor/UI/TreeViewPanel/EntityTreeStyle.h"
#include "Core/Global/ServiceLocator.h"
#include "Core/SceneSystem/SceneManager.h"
#include "renderer/Context.h"
#include "pathtrace/PathTrace.h"
#include "pathtrace/Scene.h"
#include <QFileSystemModel>
#include <QAbstractItemModel>
#include <QHeaderView>
#include <QMouseEvent>
#include <string>
#include <vector>

namespace MOON {
	static bool isEntityCheckAble(const std::string& name) {
		if (name == "HeadLight" || name == "PointLight1" || name == "PointLight2" || name == "PointLight3" || name == "PointLight4") {
			return false;
		}
		return true;

	}
	class TreeViewPanel::TreeViewPanelInternal {
	public:
		TreeViewPanelInternal(TreeViewPanel* tree) :mSelf(tree) {
			mModel = new EntityTreeModel(mSelf);
		}
		~TreeViewPanelInternal() {

		}
	private:
		friend TreeViewPanel;
		EntityTreeModel* mModel = nullptr;
		TreeViewPanel* mSelf = nullptr;
	};
	TreeViewPanel::TreeViewPanel(QWidget* parent) :QTreeView(parent), mInternal(new TreeViewPanelInternal(this))
	{
		RegService(TreeViewPanel, *this);
		QSizePolicy sizePolicy8(QSizePolicy::Preferred, QSizePolicy::Expanding);
		sizePolicy8.setHorizontalStretch(0);
		sizePolicy8.setVerticalStretch(0);
		sizePolicy8.setWidthForHeight(true);
		sizePolicy8.setHeightForWidth(this->sizePolicy().hasHeightForWidth());
		this->setSizePolicy(sizePolicy8);
		this->setModel(mInternal->mModel);
		//this->setItemDelegate(new EntityTreeViewStyleDelegate(this));
		this->header()->hide();
		this->setStyleSheet(
			"QTreeView::indicator:checked {image: url(:/entityTree/icons/pqEyeball.svg);}"
			"QTreeView::indicator:unchecked {image: url(:/entityTree/icons/pqEyeballClosed.svg);}"
		);

	}
	TreeViewPanel::~TreeViewPanel()
	{
		delete mInternal;
	}

	void TreeViewPanel::updateTreeViewSceneRoot() {
		mInternal->mModel->onSceneRootChange();
	}
	void TreeViewPanel::updateTreeViewPathRoot()
	{
		mInternal->mModel->onPathRootChange();
	}

	void TreeViewPanel::mousePressEvent(QMouseEvent* event)
	{
		QPoint mousePos = event->pos();

		QTreeView::mousePressEvent(event);
		QModelIndex index = indexAt(mousePos);
		if (!index.isValid()) return;
		QRect itemRect = visualRect(index);


		// 获取item在视图中的矩形

		if (!itemRect.contains(mousePos)) {

			return;
		}
		QStyleOptionViewItem option = viewOptions();
		option.rect = itemRect;
		option.index = index;
		QRect textRect = style()->subElementRect(QStyle::SE_ItemViewItemText, &option, this);
		QPoint posInItem = mousePos;
		if (textRect.contains(posInItem)) {
			::Core::ECS::Actor* actor = static_cast<::Core::ECS::Actor*>(index.data(Qt::UserRole).value<void*>());
			if (actor) {
				if (isEntityCheckAble(actor->GetName())) {
                   emit setSelectActor(actor);
				}
				
			}
			
		}
	}



}