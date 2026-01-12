#pragma once
#include "PropertyWidget.h"
#include "core/ECS/Actor.h"
#include "core/ECS/Components/CMaterialRenderer.h"
#include "Core/ECS/Components/CPostProcessStack.h"
#include "core/Resources/Material.h"
#include "Core/Global/ServiceLocator.h"
#include "editor/UI/PropertyPanel/PropertyModel.h"
#include "editor/UI/PropertyPanel/Collapsiblegroupboxwidget.h"
#include "editor/UI/PropertyPanel/Property.h"
#include "Widgets/sliderwidget.h"
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
		virtual PropertyQtWidget* createEditorWidget(QWidget* parent = nullptr)override {
			if (widget == nullptr) {
				widget = new Fvec3(parent,this);
				widget->setVec3Value(owner->getPropertyValue(mName).value<Maths::FVector3>());
			}
			return widget;
		}
		virtual void setOwnerPropertyValue(const QVariant& value)override {
			owner->setPropertyValue(mName, value);
		}
		virtual void onWidgetValueChange()override {
			setOwnerPropertyValue(QVariant::fromValue(widget->getVec3Value()));
		}
		virtual void updateWidgetValue(const QVariant& value)override {
			widget->setVec3Value(value.value<Maths::FVector3>());
		}
	private:
		Fvec3* widget = nullptr;
	};
	class SliderFloatProperty :public Property {
	public:
		SliderFloatProperty(const QString& n, ActorPropertyComponent* comp) :Property(n, comp) {

		}
		~SliderFloatProperty() {

		}
		virtual PropertyQtWidget* createEditorWidget(QWidget* parent = nullptr)override {
			if (widget == nullptr) {
				widget = new FloatSliderWidgetQt(parent);
				widget->setProp(this);
				widget->setValue(owner->getPropertyValue(mName).toFloat());
				widget->setMinValue(-10.0f);
				widget->setMaxValue(10.0f);
				widget->setIncrement(0.05f);
			}
			return widget;
		}
		virtual void setOwnerPropertyValue(const QVariant& value)override {
			owner->setPropertyValue(mName, value);
		}
		virtual void onWidgetValueChange()override {
			setOwnerPropertyValue(QVariant::fromValue(widget->getValue()));
		}
		virtual void updateWidgetValue(const QVariant& value)override {
			//widget->setVec3Value(value.value<Maths::FVector3>());
		}
	private:
		FloatSliderWidgetQt* widget = nullptr;
	};
	class SliderIntProperty :public Property {
	public:
		SliderIntProperty(const QString& n, ActorPropertyComponent* comp) :Property(n, comp) {

		}
		~SliderIntProperty() {

		}
		virtual PropertyQtWidget* createEditorWidget(QWidget* parent = nullptr)override {
			if (widget == nullptr) {
				widget = new IntSliderWidgetQt(parent);
				widget->setProp(this);
				widget->setValue(owner->getPropertyValue(mName).toInt());
				widget->setMinValue(-10);
				widget->setMaxValue(10);
				widget->setIncrement(1);
			}
			return widget;
		}
		virtual void setOwnerPropertyValue(const QVariant& value)override {
			owner->setPropertyValue(mName, value);
		}
		virtual void onWidgetValueChange()override {
			setOwnerPropertyValue(QVariant::fromValue(widget->getValue()));
		}
		virtual void updateWidgetValue(const QVariant& value)override {
			//widget->setVec3Value(value.value<Maths::FVector3>());
		}
	private:
		IntSliderWidgetQt* widget = nullptr;
	};
	class PostProcessStackPropertyComponent :public ActorPropertyComponent
	{
	public:
		PostProcessStackPropertyComponent(Core::ECS::Components::CPostProcessStack* comp) :ActorPropertyComponent(comp) {
			mProperties.push_back(new SliderFloatProperty("Bloom Intensity", this));
			mProperties.push_back(new SliderIntProperty("Bloom Pass Count", this));
		}
		virtual ~PostProcessStackPropertyComponent() {

		}
		virtual QVariant getPropertyValue(const QString& propertyName)override {
			auto comp = dynamic_cast<Core::ECS::Components::CPostProcessStack*>(component);
			auto bloomSetting=comp->GetBloomSettings();
			if (propertyName == "Bloom Intensity")
				return QVariant::fromValue(bloomSetting.intensity);
			else if (propertyName == "Bloom Pass Count")
				return QVariant::fromValue(bloomSetting.passes);
			else if (propertyName == "rotation") {
				
			}
			return QVariant();
		}
		virtual void setPropertyValue(const QString& propertyName, const QVariant& value)override {
			auto comp = dynamic_cast<Core::ECS::Components::CPostProcessStack*>(component);
			auto bloomSetting = comp->GetBloomSettings();
			if (propertyName == "Bloom Intensity") {
				bloomSetting.intensity = value.value<float>();
				
			}
			else if (propertyName == "Bloom Pass Count") {
				bloomSetting.passes= value.value<int>();
			}
			else if (propertyName == "rotation") {
			}
			comp->SetBloomSettings(bloomSetting);
		}
	};
	class TransFormPropertyComponent:public ActorPropertyComponent
	{
	public:
		TransFormPropertyComponent(Core::ECS::Components::CTransform* comp):ActorPropertyComponent(comp) {
			mProperties.push_back(new FVec3Property("position", this));
			mProperties.push_back(new FVec3Property("scale", this));
			mProperties.push_back(new FVec3Property("rotation", this));
		}
		virtual ~TransFormPropertyComponent() {

		}
		virtual QVariant getPropertyValue(const QString& propertyName)override {
			auto comp = dynamic_cast<Core::ECS::Components::CTransform*>(component);
			if (propertyName == "position")
				return QVariant::fromValue(comp->GetWorldPosition());
			else if (propertyName == "scale")
				return QVariant::fromValue(comp->GetWorldScale());
			else if (propertyName == "rotation") {
				return QVariant::fromValue(comp->GetWorldRotation().EulerAngles());
			}
			return QVariant();
		}
		virtual void setPropertyValue(const QString& propertyName, const QVariant& value)override {
			auto comp = dynamic_cast<Core::ECS::Components::CTransform*>(component);
			if (propertyName == "position") {
				comp->SetWorldPosition(value.value<Maths::FVector3>());
			}
			else if (propertyName == "scale") {
				comp->SetWorldScale(value.value<Maths::FVector3>());
			}
			else if (propertyName == "rotation") {
				auto euler=value.value<Maths::FVector3>();
				comp->SetWorldRotation(Maths::FQuaternion(euler));
			}
		}
	};
	class MaterialPropertyComponent :public ActorPropertyComponent
	{
	public:
		MaterialPropertyComponent(Core::ECS::Components::CMaterialRenderer* comp) :ActorPropertyComponent(comp) {
			mat = comp->GetMaterialAtIndex(0);
			for (auto& mprop:mat->GetProperties()) {
				if (std::holds_alternative<Maths::FVector3>(mprop.second.value)) {
					//auto v = std::get<Maths::FVector3>(mprop.second.value);
					mProperties.push_back(new FVec3Property(mprop.first.c_str(), this));
				}
				else if (std::holds_alternative<float>(mprop.second.value)) {
					//auto v = std::get<Maths::FVector3>(mprop.second.value);
					mProperties.push_back(new SliderFloatProperty(mprop.first.c_str(), this));
				}
			}
		}
		virtual ~MaterialPropertyComponent() {

		}
		virtual QVariant getPropertyValue(const QString& propertyName)override {
			auto ref = mat->GetProperty(propertyName.toStdString());
			if (ref.has_value()) {
				if (std::holds_alternative<Maths::FVector3>(ref.value().value)) {
					auto v = std::get<Maths::FVector3>(ref.value().value);
					return QVariant::fromValue(v);
				}
				else if (std::holds_alternative<float>(ref.value().value)) {
					auto v = std::get<float>(ref.value().value);
					return QVariant::fromValue(v);
				}
			}
			return QVariant();
		}
		virtual void setPropertyValue(const QString& propertyName, const QVariant& value)override {
			
			if (mat->HasProperty(propertyName.toStdString())) {
				if (value.canConvert<Maths::FVector3>()) {
					mat->SetProperty(propertyName.toStdString(),value.value<Maths::FVector3>());
				}
				else if (value.canConvert<float>()) {
					mat->SetProperty(propertyName.toStdString(), value.value<float>());
				}
			}
		}
	private:
		Core::Resources::Material* mat;
	};
	ActorPropertyComponent* transferActorPropertyComponent(Core::ECS::Components::AComponent* comp) {

		if (auto trans = dynamic_cast<Core::ECS::Components::CTransform*>(comp)) {
			return new  TransFormPropertyComponent(trans);
		}
		else if (auto trans = dynamic_cast<Core::ECS::Components::CMaterialRenderer*>(comp)) {
			return new  MaterialPropertyComponent(trans);
		}
		else if (auto trans = dynamic_cast<Core::ECS::Components::CPostProcessStack*>(comp)) {
			return new  PostProcessStackPropertyComponent(trans);
		}
		return new ActorPropertyComponent(comp);
	
	}
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
						auto p = transferActorPropertyComponent(ptr.get());
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