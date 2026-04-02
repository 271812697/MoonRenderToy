#pragma once
#include "core/SceneSystem/BvhService.h"
#include "editor/UI/SettingPanel/PassSettingWidget.h"
#include "Core/Global/ServiceLocator.h"
#include "Settings/DebugSetting.h"
#include "editor/UI/PropertyPanel/Collapsiblegroupboxwidget.h"
#include "renderer/SceneView.h"
#include "Rendering/Core/ARenderPass.h"
#include "renderer/DebugSceneRenderer.h"
#include "renderer/SceneView.h"
#include "Widgets/PropertyComponent.h"
#include "Widgets/Property.h"
#include "Widgets/BoolProperty.h"
#include "Widgets/FVec3Property.h"
#include "Widgets/EnumProperty.h"
#include "Widgets/SliderFloatProperty.h"
#include "Widgets/SliderIntProperty.h"
#include "Widgets/ColorPickerProperty.h"
#include "Core/Rendering/GbufferPass.h"
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

	class RenderPassComponent :public PropertyComponent
	{
	public:
		RenderPassComponent(Rendering::Core::ARenderPass* p) :pass(p) {
			mProperties.push_back(new BoolProperty("Enable", this));
			
		}
		virtual ~RenderPassComponent() {
		}
		virtual QVariant getPropertyValue(const QString& propertyName)override {
			if (propertyName == "Enable") {
				return QVariant::fromValue(pass->IsEnabled());
			}
			return QVariant();
		}
		virtual void setPropertyValue(const QString& propertyName, const QVariant& value)override {
			if (propertyName == "Enable") {
				pass->SetEnabled(value.value<bool>());
			}
		}
		virtual QString getComponentName()override {
			return "ABaseRenderPass";
		}
	protected:
		Rendering::Core::ARenderPass* pass = nullptr;
	};
	class GbufferPassComponent :public RenderPassComponent
	{
	public:
		GbufferPassComponent(::Core::Rendering::GbufferPass* p) :RenderPassComponent(p) {
			mProperties.push_back(new SliderFloatProperty("SSAO Radius", this, 0.1f, 10.0f));
			mProperties.push_back(new SliderFloatProperty("SSAO Bias", this, 0.001f, 10.0f));
		}
		virtual QVariant getPropertyValue(const QString& propertyName)override {
			auto gbpass = dynamic_cast<::Core::Rendering::GbufferPass*>(pass);
			if (propertyName == "Enable") {
				return QVariant::fromValue(pass->IsEnabled());
			}
			else if (propertyName == "SSAO Radius") {
				return QVariant::fromValue(gbpass->getGbufferParam().ssaoParam.radius);
			}
			else if (propertyName == "SSAO Bias") {
				return QVariant::fromValue(gbpass->getGbufferParam().ssaoParam.bias);
			}
			return QVariant();
		}
		virtual void setPropertyValue(const QString& propertyName, const QVariant& value)override {
			auto gbpass = dynamic_cast<::Core::Rendering::GbufferPass*>(pass);
			if (propertyName == "Enable") {
				pass->SetEnabled(value.value<bool>());
			}
			else if (propertyName == "SSAO Radius") {
				gbpass->getGbufferParam().ssaoParam.radius = value.value<float>();
			}
			else if (propertyName == "SSAO Bias") {
				gbpass->getGbufferParam().ssaoParam.bias = value.value<float>();
			}
		}
	};

	RenderPassComponent* CreateRenderPassComponent(Rendering::Core::ARenderPass* pass) {
		if (dynamic_cast<::Core::Rendering::GbufferPass*>(pass)) {
			return new GbufferPassComponent(dynamic_cast<::Core::Rendering::GbufferPass*>(pass));
		}
		else {
			return new RenderPassComponent(pass);
		}
	}


	class RenderPassSettingWidget::RenderPassSettingWidgetInternal {
	public:
		RenderPassSettingWidgetInternal(RenderPassSettingWidget* tree) :mSelf(tree) {

		}		
		void setUp() {
			// 布局
			layout_ = new QVBoxLayout(mSelf);
			layout_->setContentsMargins(0, 0, 0, 0);
			mSelf->setLayout(layout_);
			//Refresh();
		}
		void Refresh() {
			while (auto item = layout_->takeAt(0)) {
				delete item;
			}
			for (auto p : m_comps) {
				delete p.first;
				delete p.second;
			}	
			m_comps.clear();
			auto& renderer=GetService(Editor::Panels::SceneView).GetRenderer();
			auto& passMap=renderer.GetPasses();
			for (const auto& pass : passMap | std::views::values)
			{
				auto p = CreateRenderPassComponent(pass.second.get());
				
				auto collpase = new CollapsibleGroupBoxWidget(QString::fromStdString(pass.first), mSelf);
				layout_->addWidget(collpase);
				for (auto u : p->getProperties()) {
					collpase->addProperty(u);
				}
				m_comps.push_back({ collpase ,p });
			}
			layout_->addStretch();
		}
		~RenderPassSettingWidgetInternal() {
		}
	private:
		friend class RenderPassSettingWidget;
		RenderPassSettingWidget* mSelf = nullptr;
		QVBoxLayout* layout_ = nullptr;
		
		std::vector<std::pair<CollapsibleGroupBoxWidget*, PropertyComponent*>>m_comps;
	};
	RenderPassSettingWidget::RenderPassSettingWidget(QWidget* parent):QWidget(parent),mInternal(new RenderPassSettingWidgetInternal(this))
	{
		RegService(RenderPassSettingWidget, *this);
		mInternal->setUp();
	}
	void RenderPassSettingWidget::Refresh() {
		mInternal->Refresh();
	}
	RenderPassSettingWidget::~RenderPassSettingWidget()
	{
		delete mInternal;
	}
}