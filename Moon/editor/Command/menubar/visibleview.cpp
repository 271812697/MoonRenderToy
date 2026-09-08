#include "visibleview.h"
#include "editor/UI/TreeViewPanel/hierarchypanel.h"
#include "editor/UI/SettingPanel/SettingPanel.h"
#include "editor/UI/PropertyPanel/PropertyPanel.h"
#include "editor/UI/TaskPanel/TaskViewPanel.h"
#include "editor/UI/LogPanel/LogPanel.h"
#include "Core/Global/ServiceLocator.h"
#include "core/log.h"
#include "renderer/SceneView.h"

#include <QMenu>
#include <QtWidgets/QFileDialog>
#include <QCoreApplication>


namespace MOON {
	VisibleViewCommand::VisibleViewCommand(QObject* parent):QObject(parent)
	{
		
	}
	void VisibleViewCommand::setUp(QMenu* menu)
	{
		auto hierarchypanel = new QAction(VisibleViewCommand::tr("Hierarchy"), this);
		auto settingPanel = new QAction(VisibleViewCommand::tr("Setting"), this);
		auto propertyPanel = new QAction(VisibleViewCommand::tr("Property"), this);
		auto taskViewPanel = new QAction(VisibleViewCommand::tr("Task View"), this);
		auto logPanel = new QAction(VisibleViewCommand::tr("Log"), this);
		hierarchypanel->setCheckable(true);
		hierarchypanel->setChecked(true);
		settingPanel->setCheckable(true);
		settingPanel->setChecked(true);
		propertyPanel->setCheckable(true);
		propertyPanel->setChecked(true);
		taskViewPanel->setCheckable(true);
		taskViewPanel->setChecked(true);
		logPanel->setCheckable(true);
		logPanel->setChecked(true);
		menu->addAction(hierarchypanel);
		menu->addAction(settingPanel);
		menu->addAction(propertyPanel);
		menu->addAction(taskViewPanel);
		menu->addAction(logPanel);

		connect(hierarchypanel, &QAction::triggered, [](bool check) {
			if (check) {
				GetService(MOON::Hierarchypanel).show();
			}
			else
			{
				GetService(MOON::Hierarchypanel).hide();
			}

			});
		connect(settingPanel, &QAction::triggered, [](bool check) {
			if (check) {
				GetService(MOON::SettingPanel).show();
			}
			else
			{
				GetService(MOON::SettingPanel).hide();
			}

			});
		connect(propertyPanel, &QAction::triggered, [](bool check) {
			if (check) {
				GetService(MOON::PropertyPanel).show();
			}
			else
			{
				GetService(MOON::PropertyPanel).hide();
			}

			});
		connect(taskViewPanel, &QAction::triggered, [](bool check) {
			if (check) {
				GetService(MOON::TaskViewPanel).show();
			}
			else
			{
				GetService(MOON::TaskViewPanel).hide();
			}

			});
		connect(logPanel, &QAction::triggered, [](bool check) {
			if (check) {
				GetService(MOON::LogPanel).show();
			}
			else
			{
				GetService(MOON::LogPanel).hide();
			}

			});

	}
}




