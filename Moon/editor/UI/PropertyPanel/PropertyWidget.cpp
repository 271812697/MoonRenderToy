#pragma once
#include "PropertyWidget.h"
#include "core/ECS/Actor.h"
#include "Core/Global/ServiceLocator.h"
#include "editor/UI/PropertyPanel/PropertyModel.h"
#include <QTreeWidget>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QHBoxLayout>
#include <QHeaderView>

#include <QColorDialog>
#include <QFontDialog>
#include <QMessageBox>
#include <QScrollArea>
#include <QStyleFactory>
#include <QLabel>
#include <QGroupBox>
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <fstream>

namespace MOON {

	class PropertyWidget::PropertyWidgetInternal {
	public:
		PropertyWidgetInternal(PropertyWidget* tree) :mSelf(tree) {

		}		
		void setUp() {
			// 初始化控件
			m_treeView = new QTreeView(mSelf);
			m_propertyModel = new PropertyTreeModel(mSelf);
			m_propertyDelegate = new PropertyDelegate(mSelf);

			// 配置TreeView
			m_treeView->setModel(m_propertyModel);
			m_treeView->setItemDelegate(m_propertyDelegate);
			m_treeView->setColumnWidth(0, 200); // 设置第一列宽度
			m_treeView->header()->setSectionResizeMode(QHeaderView::Stretch); // 第二列自适应

			// 布局
			QVBoxLayout* layout = new QVBoxLayout(mSelf);
			layout->setContentsMargins(0, 0, 0, 0);
			layout->addWidget(m_treeView);
			mSelf->setLayout(layout);
		}
		~PropertyWidgetInternal() {
		}
		// 设置选中的Actor（核心入口）
		void setSelectedActor(Core::ECS::Actor* actor) {
			if (actor != m_selectedActor) {
				// 记录当前选中的Actor
				m_selectedActor = actor;
				// 构建新面板
				if (m_selectedActor) {
					m_propertyModel->setCurrentActor(actor);
					// 自动展开所有节点
					m_treeView->expandAll();
				}
			}
		}
	private:
		friend class PropertyWidget;
		PropertyWidget* mSelf = nullptr;
		QTreeView* m_treeView;
		PropertyTreeModel* m_propertyModel;
		PropertyDelegate* m_propertyDelegate;
		Core::ECS::Actor* m_selectedActor=nullptr;          // 当前选中的Actor

	};
	PropertyWidget::PropertyWidget(QWidget* parent):QWidget(parent),mInternal(new PropertyWidgetInternal(this))
	{
		RegService(PropertyWidget, *this);
		mInternal->setUp();

	}
	void PropertyWidget::setSelectedActor(Core::ECS::Actor* actor) {
		mInternal->setSelectedActor(actor);
	}
	PropertyWidget::~PropertyWidget()
	{
		delete mInternal;
	}


}