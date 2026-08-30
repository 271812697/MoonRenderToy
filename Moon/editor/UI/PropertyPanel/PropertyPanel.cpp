#pragma once
#include "PropertyPanel.h"
#include "PropertyWidget.h"
#include "editor/UI/DockWidgetTitleBar.h"
#include "Core/Global/ServiceLocator.h"
#include "Widgets/utils.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QScrollArea>
namespace MOON {


	PropertyPanel::PropertyPanel(QWidget* parent):QDockWidget(parent)
	{
		RegService(PropertyPanel, *this);
		setTitleBarWidget(new DockWidgetTitleBar(this));
		auto scrollArea_ = new QScrollArea();
		scrollArea_->setWidgetResizable(true);
		scrollArea_->setMinimumWidth(emToPx(this, 30));
		scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrollArea_->setFrameShape(QFrame::NoFrame);
		scrollArea_->setContentsMargins(0, 0, 0, 0);

		PropertyWidget* ui =new PropertyWidget(this);
		scrollArea_->setWidget(ui);
		setWidget(scrollArea_);
	}
}
