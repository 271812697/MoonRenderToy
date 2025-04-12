#include "pqLoadDataReaction.h"
#include "pqFileDialog.h"
#include <QDebug>
#include <QInputDialog>
#include <QMap>
#include <QRegExp>
#include <QApplication>
#include <QMainWindow>

#include <algorithm>
#include <cassert>
#include <string>
#include <vector>
#include <iostream>

//-----------------------------------------------------------------------------
pqLoadDataReaction::pqLoadDataReaction(QAction* parentObject)
	: Superclass(parentObject)
{


	this->updateEnableState();
}

void pqLoadDataReaction::onTriggered()
{
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

}

//-----------------------------------------------------------------------------
void pqLoadDataReaction::updateEnableState()
{

}





