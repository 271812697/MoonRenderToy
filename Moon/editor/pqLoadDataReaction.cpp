#include "pqLoadDataReaction.h"
#include "pqFileDialog.h"
#include <QtWidgets/QFileDialog>
#include <QDebug>
#include <QInputDialog>
#include <QMap>
#include <QRegExp>
#include <QApplication>
#include <QMainWindow>
#include "editor/viewerwindow.h"
#include "renderer/Context.h"

#include "OvCore/Global/ServiceLocator.h"
#include <algorithm>
#include <cassert>
#include <string>
#include <vector>
#include <iostream>
#include "core/log.h"
#include "pathtrace/PathTrace.h"

//-----------------------------------------------------------------------------
pqLoadDataReaction::pqLoadDataReaction(QAction* parentObject)
	: Superclass(parentObject)
{


	this->updateEnableState();
}

void pqLoadDataReaction::onTriggered()
{
	/*
	QWidget* mainwidget = nullptr;
	Q_FOREACH(QWidget * widget, QApplication::topLevelWidgets())
	{
		if (widget->isWindow() && widget->isVisible() && qobject_cast<QMainWindow*>(widget))
		{
			mainwidget = widget;
			break;
		}
	}
	QString filtersString = "";
	std::cout << "Open file" << std::endl;
	pqFileDialog fileDialog(
		mainwidget, tr("Open File:"), QString(), filtersString, false);
	fileDialog.setObjectName("FileOpenDialog");
	fileDialog.setFileMode(pqFileDialog::ExistingFilesAndDirectories);
	if (fileDialog.exec() == QDialog::Accepted) {

	}

	*/


	QString fileName = QFileDialog::getOpenFileName(nullptr,
		tr("Open Flow Scene"),
		QDir::homePath(),
		tr("Flow Scene Files (*.scene;*.gltf;*.obj;*.stl)"));

	if (!QFileInfo::exists(fileName))
		return;
	CORE_INFO("Switch to Scene {0}", fileName.toStdString());
	PathTrace::SwitchScene(fileName.toStdString());
	OVSERVICE(MOON::ViewerWindow).switchScene();




}

//-----------------------------------------------------------------------------
void pqLoadDataReaction::updateEnableState()
{

}





