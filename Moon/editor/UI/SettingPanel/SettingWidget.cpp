#pragma once
#include "SettingWidget.h"
#include "OvCore/Global/ServiceLocator.h"
#include "QtWidgets/checkbox.h"
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

	class SettingWidget::SettingWidgetInternal {
	public:
		SettingWidgetInternal(SettingWidget* tree) :mSelf(tree) {
			
		}		


		void setUp() {
			// 主布局
			QHBoxLayout* mainLayout = new QHBoxLayout(mSelf);
			mainLayout->setContentsMargins(0, 0, 0, 0);
			mainLayout->setSpacing(0);
			// 左侧导航树
			m_navTree = new QTreeWidget(mSelf);
			m_navTree->setHeaderHidden(true);
			m_navTree->setMaximumWidth(200);
			m_navTree->setMinimumWidth(180);
			m_navTree->setStyleSheet("QTreeWidget { border-right: 1px solid #ddd; }"
				"QTreeWidget::item { height: 30px; padding-left: 10px; }"
				"QTreeWidget::item:selected { background-color: #e6f7ff; color: #1890ff; }");




			// 右侧内容区域
			m_contentStack = new QStackedWidget(mSelf);


			// 添加到主布局
			mainLayout->addWidget(m_navTree);
			mainLayout->addWidget(m_contentStack, 1);
			// 连接导航切换信号
			connect(m_navTree, &QTreeWidget::currentItemChanged,
				mSelf, &SettingWidget::onItemChanged);
		}
		~SettingWidgetInternal() {
		}
	private:
		friend class SettingWidget;
		SettingWidget* mSelf = nullptr;
		// 创建各个设置页面

		// 左侧导航树
		QTreeWidget* m_navTree;

		// 右侧内容区域
		QStackedWidget* m_contentStack;

	};
	SettingWidget::SettingWidget(QWidget* parent):QWidget(parent),mInternal(new SettingWidgetInternal(this))
	{
		mInternal->setUp();

	}
	SettingWidget::~SettingWidget()
	{
		delete mInternal;
	}
	void SettingWidget::onItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous) {
		Q_UNUSED(previous);
		int index = mInternal->m_navTree->indexOfTopLevelItem(current);
		if (index >= 0) {
			mInternal->m_contentStack->setCurrentIndex(index);
		}
	}

}