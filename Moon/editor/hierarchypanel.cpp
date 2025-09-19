#pragma once
#include "hierarchypanel.h"
#include "editor/UI/TreeViewPanel/treeViewpanel.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>


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
		QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

		sizePolicy.setHeightForWidth(this->sizePolicy().hasHeightForWidth());
		this->setSizePolicy(sizePolicy);
	}
}