#pragma once
#include "PropertyWidget.h"
#include "core/ECS/Actor.h"
#include "Core/Global/ServiceLocator.h"
#include "editor/UI/PropertyPanel/PropertyModel.h"
#include "editor/UI/PropertyPanel/Collapsiblegroupboxwidget.h"
#include "editor/UI/PropertyPanel/Property.h"
#include "Widgets/FVec3.h"
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
	class FVec3Property :public Property {
	public:
		FVec3Property(const QString& n , ActorPropertyComponent* comp):Property(n,comp){

		}
		~FVec3Property() {

		}
		virtual QWidget* createEditorWidget(QWidget* parent = nullptr)override {
			if (widget == nullptr) {
				widget = new Fvec3(parent,this);
				widget->setVec3Value(owner->getPropertyValue(mName).value<Maths::FVector3>());
			}
			return widget;
		}
		virtual void setPropertyValue(const QVariant& value)override {
			owner->setPropertyValue(mName, value);
		}
		virtual void onPropertyValueChange()override {
			setPropertyValue(QVariant::fromValue(widget->getVec3Value()));
		}
		virtual void updateWidgetValue(const QVariant& value)override {
			widget->setVec3Value(value.value<Maths::FVector3>());
		}
	private:
		Fvec3* widget = nullptr;
	};
	class TransFormPropertyComponent:public ActorPropertyComponent
	{
	public:
		TransFormPropertyComponent(Core::ECS::Components::CTransform* comp):ActorPropertyComponent(comp) {
			mProperties.push_back(new FVec3Property("position", this));
			mProperties.push_back(new FVec3Property("scale", this));
		}
		virtual ~TransFormPropertyComponent() {

		}
		virtual QVariant getPropertyValue(const QString& propertyName)override {
			auto comp = dynamic_cast<Core::ECS::Components::CTransform*>(component);
			if (propertyName == "position")
				return QVariant::fromValue(comp->GetWorldPosition());
			if (propertyName == "scale")
				return QVariant::fromValue(comp->GetWorldScale());
			return QVariant();
		}
		virtual void setPropertyValue(const QString& propertyName, const QVariant& value)override {
			auto comp = dynamic_cast<Core::ECS::Components::CTransform*>(component);
			if (propertyName == "position") {
				comp->SetWorldPosition(value.value<Maths::FVector3>());
			}
			if (propertyName == "scale") {
				comp->SetWorldScale(value.value<Maths::FVector3>());
			}
		}
	};
	class PropertyWidget::PropertyWidgetInternal {
	public:
		PropertyWidgetInternal(PropertyWidget* tree) :mSelf(tree) {

		}		
		void setUp() {
			// 布局
			layout_ = new QVBoxLayout(mSelf);
			layout_->setContentsMargins(0, 0, 0, 0);
			// 初始化控件
			//m_treeView = new QTreeView(mSelf);
			//m_treeView->setRootIsDecorated(false);
			//m_propertyModel = new PropertyTreeModel(mSelf);
			//m_propertyDelegate = new PropertyDelegate(mSelf);
			////m_coll = new CollapsibleGroupBoxWidget(mSelf);
			//
			//// 配置TreeView
			//m_treeView->setModel(m_propertyModel);
			//m_treeView->setItemDelegate(m_propertyDelegate);
			//
			//
			//m_treeView->setColumnWidth(0, 50); // 设置第一列宽度
			//m_treeView->header()->setSectionResizeMode(QHeaderView::Stretch); // 第二列自适应

			//layout_->addWidget(m_treeView);
			//layout->addWidget(m_coll);
			mSelf->setLayout(layout_);
		}
		~PropertyWidgetInternal() {
			for (auto p : m_comps) {
				delete p.second;
			}
		}
		// 设置选中的Actor（核心入口）
		void setSelectedActor(Core::ECS::Actor* actor) {
			if (actor != m_selectedActor) {
				// 记录当前选中的Actor
				m_selectedActor = actor;
				// 构建新面板
				if (m_selectedActor) {
					//m_propertyModel->setCurrentActor(actor);
					// 自动展开所有节点
					//m_treeView->expandAll();
					//m_treeView->setEditTriggers(QAbstractItemView::AllEditTriggers);
					//m_treeView->openPersistentEditor(m_treeView->rootIndex());
					while (auto item = layout_->takeAt(0)) {
						delete item;
					}
					for (auto p : m_comps) {
						delete p.first;
						delete p.second;
					}
					m_comps.clear();
					for (auto& ptr : m_selectedActor->GetComponents()) {
						auto actorComp = ptr.get();
						auto trans = dynamic_cast<Core::ECS::Components::CTransform*>(actorComp);
						auto p = trans ? new  TransFormPropertyComponent(trans) : new ActorPropertyComponent(ptr.get());
						auto collpase = new CollapsibleGroupBoxWidget(p->getComponentName(), mSelf);
						layout_->addWidget(collpase);
						for (auto u:p->getProperties()) {
							collpase->addProperty(u);
						}
						m_comps.push_back({ collpase ,p});
					}
					layout_->addStretch();
				}
			}
		}
	private:
		friend class PropertyWidget;
		PropertyWidget* mSelf = nullptr;
		QVBoxLayout* layout_ = nullptr;
		QTreeView* m_treeView;
		
		PropertyTreeModel* m_propertyModel;
		PropertyDelegate* m_propertyDelegate;
		Core::ECS::Actor* m_selectedActor=nullptr;          // 当前选中的Actor
		std::vector<std::pair<CollapsibleGroupBoxWidget*, ActorPropertyComponent*>>m_comps;
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