#pragma once
#include "PropertyPanel.h"
#include "PropertyWidget.h"
#include "Core/Global/ServiceLocator.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QScrollArea>
namespace MOON {
	int emToPPx(const QFontMetrics& m, double em) {
		const auto pxPerEm = m.boundingRect(QString(100, 'M')).width() / 100.0;
		return static_cast<int>(std::round(pxPerEm * em));
	}
	int emToPx(const QWidget* w, double em) {
		w->ensurePolished();
		return emToPPx(w->fontMetrics(), em);
	}


	PropertyPanel::PropertyPanel(QWidget* parent):QDockWidget(parent)
	{
		RegService(PropertyPanel, *this);
		auto scrollArea_ = new QScrollArea();
		scrollArea_->setWidgetResizable(true);
		scrollArea_->setMinimumWidth(emToPx(this, 30));
		scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrollArea_->setFrameShape(QFrame::NoFrame);
		scrollArea_->setContentsMargins(0, 0, 0, 0);

		PropertyWidget* ui =new PropertyWidget(this);
		scrollArea_->setWidget(ui);
		setWidget(scrollArea_);
	}
}