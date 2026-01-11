#pragma once
#include "SettingWidget.h"
#include "Core/Global/ServiceLocator.h"
#include "Settings/DebugSetting.h"
#include "Widgets/checkbox.h"
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


			auto& group=DebugSettings::instance().getGroup();
			auto& nodes = DebugSettings::instance().getRegistry();
			// 添加导航项
			for (auto& g : group) {
				QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << QString::fromStdString(g.first));
				m_navTree->addTopLevelItem(item);
			}
			// 右侧内容区域
			m_contentStack = new QStackedWidget(mSelf);

			// 添加各个设置页面
			for (auto& g : group) {
				// 实现高级设置页面
				QScrollArea* scrollArea = new QScrollArea();
				scrollArea->setWidgetResizable(true);
				scrollArea->setStyleSheet("QScrollArea { background-color: #f5f5f5; }");

				QWidget* contentWidget = new QWidget();
				QVBoxLayout* vLayout = new QVBoxLayout(contentWidget);
				vLayout->setContentsMargins(0, 0, 0, 0);
				vLayout->setSpacing(0);
				vLayout->setAlignment(Qt::AlignTop);

				// 高级选项分组
				QWidget* advancedGroup = new QWidget();
				//advancedGroup->setStyleSheet("QGroupBox { font-size: 14px; font-weight: bold; color: #555; border: 1px solid #ddd; border-radius: 6px; margin-top: 10px; padding: 15px; }"
					///"QGroupBox::title { subcontrol-origin: margin; left: 10px; }");

				QFormLayout* form = new QFormLayout(advancedGroup);
				form->setRowWrapPolicy(QFormLayout::DontWrapRows);
				form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
				form->setSpacing(15);

				form->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
				// 2. 设置字段的对齐方式（顶部对齐）
				form->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
				// 3. 可选：设置行内间距（避免控件太紧凑）
				form->setVerticalSpacing(10); // 行与行之间的垂直间距
				form->setContentsMargins(20, 20, 20, 20); // 布局边缘间距
				for (int idx : g.second) {

					QLabel* label = new QLabel(QString::fromStdString(nodes[idx]->getName()));
					QWidget* widget=nodes[idx]->createWidget(advancedGroup);
					if (widget) {
						form->addRow(label, widget);
					}
				}
				vLayout->addWidget(advancedGroup);
				scrollArea->setWidget(contentWidget);
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