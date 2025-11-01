#include "visibleview.h"
#include "editor/UI/ReousrcePanel/resourcePanel.h"
#include "editor/UI/TreeViewPanel/hierarchypanel.h"
#include "editor/UI/SettingPanel/SettingPanel.h"
#include "OvCore/Global/ServiceLocator.h"
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
		auto resource = new QAction(VisibleViewCommand::tr("Resource"), this);
		auto hierarchypanel = new QAction(VisibleViewCommand::tr("Hierarchy"), this);
		auto settingPanel = new QAction(VisibleViewCommand::tr("Setting"), this);
		resource->setCheckable(true);
		resource->setChecked(true);
		hierarchypanel->setCheckable(true);
		hierarchypanel->setChecked(true);
		settingPanel->setCheckable(true);
		settingPanel->setChecked(true);
		menu->addAction(resource);
		menu->addAction(hierarchypanel);
		menu->addAction(settingPanel);

		connect(resource, &QAction::triggered, [](bool check) {
			if (check) {
               GetService(MOON::ResPanel).show();
			}
			else
			{
			   GetService(MOON::ResPanel).hide();
			}
			
			});
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

	}
}




