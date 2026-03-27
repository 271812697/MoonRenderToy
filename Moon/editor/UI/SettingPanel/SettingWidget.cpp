#pragma once
#include "SettingWidget.h"
#include "Core/Global/ServiceLocator.h"
#include "Settings/DebugSetting.h"
#include "Widgets/checkbox.h"
#include "editor/UI/SettingPanel/RenderSettingWidget.h"
#include "editor/UI/SettingPanel/DebugSettingWidget.h"
#include "Widgets/utils.h"
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
			m_navTree->setMaximumWidth(150);
			m_navTree->setMinimumWidth(100);
			m_navTree->setStyleSheet("QTreeWidget { border-right: 1px solid #ddd; }"
				"QTreeWidget::item { height: 30px; padding-left: 10px; }"
				"QTreeWidget::item:selected { background-color: #e6f7ff; color: #1890ff; }");
			
			// 右侧内容区域
			m_contentStack = new QStackedWidget(mSelf);

			//1.RenderSetting
			{
				QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << QString::fromStdString("PathTrace Mat"));
				m_navTree->addTopLevelItem(item);
				QScrollArea* scrollArea = new QScrollArea();
				scrollArea->setWidgetResizable(true);
				scrollArea->setMinimumWidth(emToPx(m_contentStack, 30));
				scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
				scrollArea->setFrameShape(QFrame::NoFrame);
				scrollArea->setContentsMargins(0, 0, 0, 0);
				scrollArea->setWidget(new RenderSettingWidget(mSelf));
				m_contentStack->addWidget(scrollArea);
			}

			//2.DebugSetting
			{
				QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << QString::fromStdString("DebugSetting"));
				m_navTree->addTopLevelItem(item);
				QScrollArea* scrollArea = new QScrollArea();
				scrollArea->setWidgetResizable(true);
				scrollArea->setMinimumWidth(emToPx(m_contentStack, 30));
				scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
				scrollArea->setFrameShape(QFrame::NoFrame);
				scrollArea->setContentsMargins(0, 0, 0, 0);
				scrollArea->setWidget(new DebugSettingWidget(mSelf));
				m_contentStack->addWidget(scrollArea);
			}


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