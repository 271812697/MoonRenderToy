#pragma once
#include "hierarchypanel.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include "treeViewpanel.h"

namespace MOON {

	Hierarchypanel::Hierarchypanel(QWidget* parent) : QWidget(parent)
	{
		QVBoxLayout* layout = new QVBoxLayout(this);

		QWidget* ui = new QWidget(this);
		QVBoxLayout* ui_layout = new QVBoxLayout(ui);
		name_list = new TreeViewPanel(ui);

		ui_layout->addWidget(name_list);
		layout->addWidget(ui);
		layout->setContentsMargins(0, 0, 0, 0);
		ui_layout->setContentsMargins(0, 0, 0, 0);
	}
}