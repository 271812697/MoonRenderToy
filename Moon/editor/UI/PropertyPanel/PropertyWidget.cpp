#pragma once
#include "PropertyWidget.h"
#include "core/ECS/Actor.h"
#include "Core/Global/ServiceLocator.h"
#include "QtWidgets/CollapsibleWidget.h"
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
			// ---------------------- 核心优化：添加滚动区域 ----------------------
			QVBoxLayout* mainLayout = new QVBoxLayout(mSelf);
			mainLayout->setContentsMargins(5, 5, 5, 5);
			mainLayout->setSpacing(5);

			// 滚动区域：内容超出时显示滚动条，不会撑满面板
			QScrollArea* scrollArea = new QScrollArea(mSelf);
			scrollArea->setWidgetResizable(true); // 自适应内容大小
			scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 关闭水平滚动
			scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded); // 垂直滚动按需显示

			// 滚动区域的内容容器（存放所有折叠面板）
			QWidget* scrollContent = new QWidget(scrollArea);
			m_rootLayout = new QVBoxLayout(scrollContent);
			m_rootLayout->setSpacing(10);
			m_rootLayout->setContentsMargins(10, 10, 10, 10);
			m_rootLayout->addStretch(); // 底部添加拉伸，防止控件占满整个滚动区域

			scrollArea->setWidget(scrollContent);
			mainLayout->addWidget(scrollArea);

			// 设置PropertyPanel的最小尺寸，保证UI稳定
			mSelf->setMinimumSize(300, 400);
			mSelf->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
		}
		~PropertyWidgetInternal() {
		}
		// 设置选中的Actor（核心入口）
		void setSelectedActor(Core::ECS::Actor* actor) {
			if (actor != m_selectedActor) {
				// 清空旧面板
				clearAllPanels();

				// 记录当前选中的Actor
				m_selectedActor = actor;

				// 构建新面板
				if (m_selectedActor) {
					// 1. 构建Actor自身属性的折叠面板
					buildActorPropertyPanel();
					// 2. 构建所有组件的折叠面板
					//buildComponentsPropertyPanels();
				}
			}
		}
		// 清空所有面板（Actor+组件）
		void clearAllPanels() {
			// 遍历删除所有子控件（折叠面板）
			QLayoutItem* item;
			while ((item = m_rootLayout->takeAt(0)) != nullptr) {
				if (item->widget()) {
					item->widget()->deleteLater();
				}
				delete item;
			}
			m_allGroupBoxes.clear(); // 清空折叠面板缓存
		}

		// 构建Actor自身属性的折叠面板
		void buildActorPropertyPanel() {
			// 1. 创建可折叠的GroupBox(
			CollapsibleWidget* actorWidget = new CollapsibleWidget(QString("Actor base"), mSelf);
			QFormLayout* formLayout = new QFormLayout();
			actorWidget->contentLayout()->addLayout(formLayout);

			// 3. 添加Actor属性控件
			// 名称
			
			QLineEdit* nameEdit = new QLineEdit(QString::fromStdString(m_selectedActor->GetName()));
			formLayout->addRow("name: ", nameEdit);
			//connect(nameEdit, &QLineEdit::textChanged, this, &PropertyPanel::onActorNameEdited);
			// 4. 添加到根布局
			m_rootLayout->addWidget(actorWidget);
			m_allGroupBoxes.append(actorWidget);
		}

	private:
		friend class PropertyWidget;
		PropertyWidget* mSelf = nullptr;
		QVBoxLayout* m_rootLayout;                // 面板根布局
		Core::ECS::Actor* m_selectedActor=nullptr;          // 当前选中的Actor
		QList<CollapsibleWidget*> m_allGroupBoxes;        // 所有折叠面板缓存

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