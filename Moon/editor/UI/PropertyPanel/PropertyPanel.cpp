#pragma once
#include "PropertyPanel.h"
#include "PropertyWidget.h"
#include "Core/Global/ServiceLocator.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>
namespace MOON {
	PropertyPanel::PropertyPanel(QWidget* parent):QDockWidget(parent)
	{
		RegService(PropertyPanel, *this);
		PropertyWidget* ui =new PropertyWidget(this);
		setWidget(ui);
	}
}