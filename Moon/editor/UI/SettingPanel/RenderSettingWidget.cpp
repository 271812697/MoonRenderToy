#pragma once
#include "core/SceneSystem/BvhService.h"
#include "editor/UI/SettingPanel/RenderSettingWidget.h"
#include "Core/Global/ServiceLocator.h"

#include "editor/UI/PropertyPanel/Collapsiblegroupboxwidget.h"
#include "renderer/SceneView.h"
#include "Widgets/PropertyComponent.h"
#include "Widgets/Property.h"
#include "Widgets/BoolProperty.h"
#include "Widgets/FVec3Property.h"
#include "Widgets/EnumProperty.h"
#include "Widgets/SliderFloatProperty.h"
#include "Widgets/SliderIntProperty.h"
#include "Widgets/ColorPickerProperty.h"
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
	class PathTraceMatComponent :public PropertyComponent
	{
	public:
		PathTraceMatComponent(Core::SceneSystem::Material* mat):material(mat) {
			mProperties.push_back(new ColorPickerProperty("BaseColor", this));
			mProperties.push_back(new SliderFloatProperty("Opacity", this, 0.0, 1.0));
			mProperties.push_back(new SliderFloatProperty("Anisotropic", this, -1.0, 1.0));
			mProperties.push_back(new SliderFloatProperty("Ior", this));
			mProperties.push_back(new ColorPickerProperty("Emssion", this));
			mProperties.push_back(new SliderFloatProperty("Roughness",this,0.0,1.0));
			mProperties.push_back(new SliderFloatProperty("Metallic", this, 0.0, 1.0));
			mProperties.push_back(new SliderFloatProperty("Clearcoat", this, 0.0, 1.0));
			mProperties.push_back(new SliderFloatProperty("ClearcoatGloss", this, 0.0, 1.0));
			mProperties.push_back(new EnumProperty("MediumType", this));
			mProperties.push_back(new SliderFloatProperty("MediumDensity", this));
			mProperties.push_back(new ColorPickerProperty("MediumColor", this));
			mProperties.push_back(new SliderFloatProperty("MediumAnisotropy",this,-1.0,1.0));
			mProperties.push_back(new EnumProperty("AlphaMode", this));
			mProperties.push_back(new SliderFloatProperty("AlphaCutOff", this, 0.0, 1.0));
		}
		virtual ~PathTraceMatComponent() {
		}
		virtual QVariant getPropertyValue(const QString& propertyName)override {
			if (propertyName == "BaseColor") {
				return QVariant::fromValue(QColor(material->baseColor.x * 255, material->baseColor.y * 255, material->baseColor.z * 255));
			}
			else if (propertyName == "MediumColor") {
				return QVariant::fromValue(QColor(material->mediumColor.x * 255, material->mediumColor.y * 255, material->mediumColor.z * 255));
			}
			else if (propertyName=="MediumDensity") {
				return QVariant::fromValue(material->mediumDensity);
			}
			else if (propertyName== "MediumAnisotropy") {
				return QVariant::fromValue(material->mediumAnisotropy);
			}
			else if (propertyName == "Emssion") {
				return QVariant::fromValue(QColor(material->emission.x * 255, material->emission.y * 255, material->emission.z * 255));
			}
			else if (propertyName== "Roughness") {
				return QVariant::fromValue(material->roughness);
			}
			else if (propertyName == "AlphaCutOff") {
				return QVariant::fromValue(material->alphaCutoff);
			}
			else if (propertyName == "Metallic") {
				return QVariant::fromValue(material->metallic);
			}
			else if (propertyName == "Clearcoat") {
				return QVariant::fromValue(material->clearcoat);
			}
			else if (propertyName == "ClearcoatGloss") {
				return QVariant::fromValue(material->clearcoatGloss);
			}
			else if (propertyName == "Opacity") {
				return QVariant::fromValue(material->opacity);
			}
			else if (propertyName=="Anisotropic") {
				return QVariant::fromValue(material->anisotropic);
			}
			else if (propertyName == "Ior") {
				return QVariant::fromValue(material->ior);
			}
			else if (propertyName == "MediumType") {
				QList<QString>list = { "None","Absorb","Scatter","Emissive" };
				return QVariant::fromValue(list);
			}
			else if (propertyName == "AlphaMode") {
				QList<QString>list = { "Opaque","Blend","Mask"};
				return QVariant::fromValue(list);
			}
			return QVariant();
		}
		virtual void setPropertyValue(const QString& propertyName, const QVariant& value)override {
			bool changeMat = false;
			if (propertyName == "BaseColor") {
				auto color = value.value<QColor>();
				material->baseColor= { color.red() / 255.0f,color.green() / 255.0f,color.blue() / 255.0f };
				changeMat = true;
			}
			else if (propertyName == "MediumColor") {
				auto color = value.value<QColor>();
				material->mediumColor = { color.red() / 255.0f,color.green() / 255.0f,color.blue() / 255.0f };
				changeMat = true;
			}
			else if (propertyName == "MediumDensity") {
				material->mediumDensity = value.value<float>();
				changeMat = true;
			}
			else if (propertyName == "MediumAnisotropy") {
				material->mediumAnisotropy = value.value<float>();
				changeMat = true;
			}
			else if (propertyName == "Emssion") {
				auto color = value.value<QColor>();
				material->emission = { color.red() / 255.0f,color.green() / 255.0f,color.blue() / 255.0f };
				changeMat = true;
			}
			else if (propertyName == "Ior") {
				auto ior = value.value<float>();
				material->ior = ior;
				changeMat = true;
			}
			else if (propertyName == "Roughness") {
				auto rough=value.value<float>();
				material->roughness = rough;
				changeMat = true;
			}
			else if (propertyName == "Clearcoat") {
				material->clearcoat = value.value<float>();
				changeMat = true;
			}
			else if (propertyName == "ClearcoatGloss") {
				material->clearcoatGloss = value.value<float>();
				changeMat = true;
			}
			else if (propertyName == "AlphaCutOff") {
				material->alphaCutoff = value.value<float>();
				changeMat = true;
			}
			else if (propertyName == "Metallic") {
				material->metallic = value.value<float>();
				changeMat = true;
			}
			else if (propertyName == "Opacity") {
				material->opacity = value.value<float>();
				changeMat = true;
			}
			else if (propertyName == "Anisotropic") {
				material->anisotropic = value.value<float>();
				changeMat = true;
			}
			else if (propertyName == "MediumType") {
				int v=value.value<int>();
				material->mediumType = v * 1.0f;
				changeMat = true;
			}
			else if (propertyName == "AlphaMode") {
				int v = value.value<int>();
				material->alphaMode= v * 1.0f;
				changeMat = true;
			}	
			if (changeMat) {
				auto& view = GetService(Editor::Panels::SceneView);
				auto bvhService = view.GetScene()->GetBvhService();
				bvhService->isMaterialDirty = true;
			}
		}
		virtual QString getComponentName()override {
			return "Material";
		}
	protected:
		Core::SceneSystem::Material* material = nullptr;
		//Core::ECS::Components::AComponent* component = nullptr;
	};
	class RenderSettingWidget::RenderSettingWidgetInternal {
	public:
		RenderSettingWidgetInternal(RenderSettingWidget* tree) :mSelf(tree) {

		}		
		void setUp() {
			// 布局
			layout_ = new QVBoxLayout(mSelf);
			layout_->setContentsMargins(0, 0, 0, 0);
			mSelf->setLayout(layout_);
		}
		void Refresh() {
			auto& view = GetService(Editor::Panels::SceneView);
			auto bvhService = view.GetScene()->GetBvhService();
			while (auto item = layout_->takeAt(0)) {
				delete item;
			}
			for (auto p : m_comps) {
				delete p.first;
				delete p.second;
			}
			m_comps.clear();
			for (int i = 0; i < bvhService->materials.size(); i++) {
				auto p = new PathTraceMatComponent(&bvhService->materials[i]);
				auto collpase = new CollapsibleGroupBoxWidget("mat", mSelf);
				layout_->addWidget(collpase);
				for (auto u : p->getProperties()) {
					collpase->addProperty(u);
				}
				m_comps.push_back({ collpase ,p });
			}
			layout_->addStretch();

		}
		~RenderSettingWidgetInternal() {
		}
	private:
		friend class RenderSettingWidget;
		RenderSettingWidget* mSelf = nullptr;
		QVBoxLayout* layout_ = nullptr;
		QTreeView* m_treeView;
		
		std::vector<std::pair<CollapsibleGroupBoxWidget*, PropertyComponent*>>m_comps;
	};
	RenderSettingWidget::RenderSettingWidget(QWidget* parent):QWidget(parent),mInternal(new RenderSettingWidgetInternal(this))
	{
		RegService(RenderSettingWidget, *this);
		mInternal->setUp();
	}
	void RenderSettingWidget::Refresh() {
	
		mInternal->Refresh();
	}
	RenderSettingWidget::~RenderSettingWidget()
	{
		delete mInternal;
	}
}