#pragma once
#include "hierarchypanel.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include "treeViewpanel.h"

namespace MOON {

	Hierarchypanel::Hierarchypanel(QWidget* parent) : QWidget(parent)
	{
		up_panel = new QWidget(this);
		up_panel_layout = new QVBoxLayout(up_panel);

		up_panel->setContentsMargins(0, 0, 0, 0);
		up_panel_layout->setContentsMargins(0, 0, 0, 0);

		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->addWidget(up_panel);

		setContentsMargins(0, 0, 0, 0);
		layout->setContentsMargins(0, 0, 0, 0);

		QWidget* ui = new QWidget(this);
		QVBoxLayout* ui_layout = new QVBoxLayout(ui);

		QWidget* buttons_widget = new QWidget(ui);
		QHBoxLayout* buttons_layout = new QHBoxLayout(buttons_widget);

		create = new QPushButton("Add", buttons_widget);
		remove = new QPushButton("Del", buttons_widget);
		rename = new QPushButton("Name", buttons_widget);
		import_ = new QPushButton("Import", buttons_widget);


		name_list = new TreeViewPanel(ui);
		//name_list->setDragDropMode(QAbstractItemView::InternalMove);

		buttons_widget->setContentsMargins(0, 0, 0, 0);
		buttons_layout->setContentsMargins(0, 0, 0, 0);
		buttons_layout->addWidget(create);
		buttons_layout->addWidget(remove);
		buttons_layout->addWidget(rename);
		buttons_layout->addWidget(import_);

		ui_layout->addWidget(buttons_widget);
		ui_layout->addWidget(name_list);

		//name_list->show();

		up_panel_layout->addWidget(ui);






	}

}