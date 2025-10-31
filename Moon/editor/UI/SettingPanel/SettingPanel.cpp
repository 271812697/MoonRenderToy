#pragma once
#include "SettingPanel.h"
#include "SettingWidget.h"
#include "OvCore/Global/ServiceLocator.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>


namespace MOON {

	SettingPanel::SettingPanel(QWidget* parent):QDockWidget(parent)
	{
		RegService(SettingPanel, *this);
		SettingWidget* ui =new SettingWidget(this);
		
		setWidget(ui);

	}
}