#pragma once
#include "treeViewpanel.h"
#include "editor/UI/TreeViewPanel/EntityTreeModel.h"
#include "editor/UI/TreeViewPanel/EntityTreeStyle.h"
#include "editor/UI/PropertyPanel/PropertyWidget.h"
#include "Core/Global/ServiceLocator.h"
#include "Core/SceneSystem/SceneManager.h"
#include "Sketcher/SketcherObj.h"
#include "Sketcher/SketcherObjManager.h"
#include "renderer/Context.h"
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
	class HighlightDelegate : public QStyledItemDelegate
	{
	public:
		explicit HighlightDelegate(TreeViewPanel* panel, QObject* parent = nullptr)
			: QStyledItemDelegate(parent), m_panel(panel) {
		}

		void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
		{
			QStyleOptionViewItem opt = option;

			if (index == m_panel->m_highlightIndex) {
				opt.state |= QStyle::State_MouseOver;
			}

			QStyledItemDelegate::paint(painter, opt, index);
		}

	private:
		TreeViewPanel* m_panel;
	};
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
		QModelIndex m_lastIndex;  // 记录上一次悬浮项
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
		this->setStyleSheet(R"(
    QTreeView::indicator:checked {
        image: url(:/entityTree/icons/pqEyeball.svg);
    }
    QTreeView::indicator:unchecked {
        image: url(:/entityTree/icons/pqEyeballClosed.svg);
    }
    QTreeView::item {
        height: 20px;
        padding-left: 4px;
    }
    QTreeView::item:hover {
        background-color: #cfe2f5;   /* 悬浮浅蓝，可自己改颜色 */
        color: #202020;
    }
    QTreeView::item:selected {
        background-color: #7ab2e8;   /* 选中颜色 */
        color: white;
    }
)");
		setMouseTracking(true);
		setFocusPolicy(Qt::StrongFocus);   // 获得焦点
		viewport()->setAttribute(Qt::WA_Hover); // 关键：让视图识别hov
		setItemDelegate(new HighlightDelegate(this, this));
		setSelectionBehavior(QAbstractItemView::SelectRows);
		setSelectionMode(QAbstractItemView::SingleSelection);

		// 👇 这一句是关键！禁止点行触发勾选
		setEditTriggers(QAbstractItemView::NoEditTriggers);
	}
	TreeViewPanel::~TreeViewPanel()
	{
		delete mInternal;
	}

	void TreeViewPanel::updateTreeViewSketcherRoot()
	{
		mInternal->mModel->onSketcherChange();
	}

	void TreeViewPanel::updateTreeViewSceneRoot() {
		mInternal->mModel->onSceneRootChange();
	}

	void TreeViewPanel::highlightByActor(Core::ECS::Actor* actor)
	{
		if (!actor) {
			clearHighlight();
			return;
		}

		auto model = qobject_cast<MOON::EntityTreeModel*>(this->model());
		if (!model) return;
		auto item=model->actorItem(actor);
		if (item) {
			QModelIndex idx = item->index();
			if (idx != m_highlightIndex) {
				// 清空旧的
				QModelIndex old = m_highlightIndex;
				m_highlightIndex = idx;
				if (old.isValid()) update(old);
				if (idx.isValid()) update(idx);

				this->expand(idx.parent());//逐级展开折叠的父节点
				this->scrollTo(idx, QAbstractItemView::PositionAtCenter);//滚动至屏幕中间
			}
		}
	}

	void TreeViewPanel::clearHighlight()
	{
		if (m_highlightIndex.isValid()) {
			QModelIndex old = m_highlightIndex;
			m_highlightIndex = QModelIndex();
			update(old);
		}
	}

	void TreeViewPanel::clearLastHoverIndex()
	{
		mInternal->m_lastIndex = QModelIndex();
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
				   GetService(PropertyWidget).setSelectedActor(actor);
                   emit setSelectActor(actor);
				}
			}
			else
			{
				SketcherObj* sketcher = static_cast<SketcherObj*>(index.data(Qt::UserRole+1).value<void*>());
				SketcherObjManager::instance().setCurrentActiveSketcherObj(sketcher);
			}
		}
	}
	void TreeViewPanel::mouseMoveEvent(QMouseEvent* event)
	{
		// 获取鼠标下的项
		QModelIndex index = indexAt(event->pos());
		if (index != mInternal->m_lastIndex) {	
			// 如果离开上一项 → 发送离开信号
			if (mInternal->m_lastIndex.isValid()) {
				::Core::ECS::Actor* lastActor = static_cast<::Core::ECS::Actor*>(mInternal->m_lastIndex.data(Qt::UserRole).value<void*>());
				emit itemLeave(lastActor);
			}
			// 如果进入新项 → 发送悬浮信号
			if (index.isValid()) {
				::Core::ECS::Actor* actor = static_cast<::Core::ECS::Actor*>(index.data(Qt::UserRole).value<void*>());
				emit itemHovered(actor);
			}
			mInternal->m_lastIndex = index;
		}
		QTreeView::mouseMoveEvent(event);
	}
}