#pragma once
#include "core/SceneSystem/BvhService.h"
#include "editor/UI/SettingPanel/DebugSettingWidget.h"
#include "Core/Global/ServiceLocator.h"
#include "Settings/DebugSetting.h"
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

	class NodeWidget : public PropertyQtWidget
	{
	public:
		explicit NodeWidget(QWidget* widget,QWidget* parent, WidgetProperty* prop):PropertyQtWidget(parent,prop){
			QHBoxLayout* mainLayout = new QHBoxLayout(this);
			mainLayout->addWidget(widget);
			mainLayout->setContentsMargins(0, 0, 0, 0);
			mainLayout->setSpacing(0);
			mainLayout->setAlignment(Qt::AlignTop|Qt::AlignLeft);
			setLayout(mainLayout);
		}
		QVariant widgetValue()override {
			return QVariant();
		}
		void setWidgetValue(const QVariant& value) override {

		}
	};
	class NodeProperty :public WidgetProperty {
	public:
		NodeProperty( NodeBase* nodebase, PropertyComponent* comp) :WidgetProperty(QString::fromStdString(nodebase->getName()), comp), node(nodebase) {
			
		}
		~NodeProperty() {

		}
		PropertyQtWidget* createEditorWidget(QWidget* parent)override {
			if (mWidget == nullptr) {
				mWidget = new NodeWidget(node->createWidget(parent), parent, this);
			}
			return mWidget;
		}
	private:
		NodeBase* node = nullptr;
	};
	class NodeComponent :public PropertyComponent
	{
	public:
		NodeComponent(const std::string& g):groupName(g) {
			auto& group = DebugSettings::instance().getGroup();
			auto& nodes = DebugSettings::instance().getRegistry();
			for (int idx : group[groupName]) {	
				mProperties.push_back(new NodeProperty( nodes[idx],this));
			}
		}
		QString getComponentName()override {
			return QString::fromStdString(groupName);
		}
	private:
		std::string groupName;
	};


	class DebugSettingWidget::DebugSettingWidgetInternal {
	public:
		DebugSettingWidgetInternal(DebugSettingWidget* tree) :mSelf(tree) {

		}		
		void setUp() {
			// 布局
			layout_ = new QVBoxLayout(mSelf);
			layout_->setContentsMargins(0, 0, 0, 0);
			mSelf->setLayout(layout_);
			Refresh();
		}
		void Refresh() {
			while (auto item = layout_->takeAt(0)) {
				delete item;
			}
			for (auto p : m_comps) {
				delete p.first;
				delete p.second;
			}	
			auto& group = DebugSettings::instance().getGroup();
			m_comps.clear();
			for (auto& g : group) {
				auto p = new NodeComponent(g.first);
				auto collpase = new CollapsibleGroupBoxWidget(p->getComponentName(), mSelf);
				layout_->addWidget(collpase);
				for (auto u : p->getProperties()) {
					collpase->addProperty(u);
				}
				m_comps.push_back({ collpase ,p });
			}

			layout_->addStretch();

		}
		~DebugSettingWidgetInternal() {
		}
	private:
		friend class DebugSettingWidget;
		DebugSettingWidget* mSelf = nullptr;
		QVBoxLayout* layout_ = nullptr;
		
		std::vector<std::pair<CollapsibleGroupBoxWidget*, PropertyComponent*>>m_comps;
	};
	DebugSettingWidget::DebugSettingWidget(QWidget* parent):QWidget(parent),mInternal(new DebugSettingWidgetInternal(this))
	{
		RegService(DebugSettingWidget, *this);
		mInternal->setUp();
	}
	void DebugSettingWidget::Refresh() {
		mInternal->Refresh();
	}
	DebugSettingWidget::~DebugSettingWidget()
	{
		delete mInternal;
	}
}