#pragma once
#include "hierarchypanel.h"
#include "editor/UI/TreeViewPanel/treeViewpanel.h"
#include "Core/Global/ServiceLocator.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>


namespace MOON {

	Hierarchypanel::Hierarchypanel(QWidget* parent) : QDockWidget(parent)
	{
		RegService(Hierarchypanel,*this);
		QWidget* ui = new QWidget(this);
		QVBoxLayout* ui_layout = new QVBoxLayout(ui);
		auto tree = new TreeViewPanel(ui);
		ui_layout->addWidget(tree);
		ui_layout->setContentsMargins(0, 0, 0, 0);
		setWidget(ui);
	}
}