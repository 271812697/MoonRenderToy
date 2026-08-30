#include "editor/UI/DockWidgetTitleBar.h"

#include <QDockWidget>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QResizeEvent>
#include <QSizePolicy>
#include <QToolButton>

#include <algorithm>

namespace MOON {

DockWidgetTitleBar::DockWidgetTitleBar(QDockWidget* parent)
    : QWidget(parent), dock_(parent) {
    setObjectName("DockWidgetTitleBar");

    label_ = new QLabel(parent->windowTitle(), this);
    label_->setObjectName("DockWidgetTitleBarLabel");
    auto labelPolicy = label_->sizePolicy();
    labelPolicy.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
    label_->setSizePolicy(labelPolicy);

    floatBtn_ = new QToolButton(this);
    floatBtn_->setObjectName("DockTitleButton");
    floatBtn_->setCheckable(true);
    floatBtn_->setChecked(parent->isFloating());
    floatBtn_->setFocusPolicy(Qt::NoFocus);

    closeBtn_ = new QToolButton(this);
    closeBtn_->setObjectName("DockTitleButton");
    closeBtn_->setFocusPolicy(Qt::NoFocus);

    const QSize iconSize(16, 16);
    QIcon floatIcon;
    floatIcon.addFile(":/darkstyle/icon_restore.png", iconSize, QIcon::Normal, QIcon::Off);
    floatIcon.addFile(":/darkstyle/icon_undock.png", iconSize, QIcon::Normal, QIcon::On);
    floatBtn_->setIcon(floatIcon);
    floatBtn_->setIconSize(iconSize);
    closeBtn_->setIcon(QIcon(":/darkstyle/icon_close.png"));
    closeBtn_->setIconSize(iconSize);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(6, 0, 2, 0);
    layout->setSpacing(0);
    layout->addWidget(label_, 1);
    layout->addWidget(floatBtn_);
    layout->addWidget(closeBtn_);

    setStyleSheet(R"(
        QWidget#DockWidgetTitleBar {
            background: #424245;
            border: none;
        }
        QLabel#DockWidgetTitleBarLabel {
            color: #e9e4de;
            background: transparent;
            font-weight: 400;
        }
        QToolButton#DockTitleButton {
            background: transparent;
            border: 1px solid transparent;
            padding: 2px;
        }
        QToolButton#DockTitleButton:hover {
            border: 1px solid #268bd2;
        }
    )");

    connect(parent, &QDockWidget::windowTitleChanged, this,
            [this](const QString&) { updateTitle(); });
    connect(parent, &QDockWidget::topLevelChanged, floatBtn_, &QToolButton::setChecked);
    connect(floatBtn_, &QToolButton::clicked, this, [this]() {
        dock_->setFloating(!dock_->isFloating());
    });
    connect(closeBtn_, &QToolButton::clicked, dock_, &QDockWidget::close);

    updateTitle();
}

DockWidgetTitleBar::~DockWidgetTitleBar() = default;

void DockWidgetTitleBar::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updateTitle();
}

void DockWidgetTitleBar::updateTitle() {
    const QString title = dock_->windowTitle();
    QFontMetrics fm(label_->font());
    label_->setText(fm.elidedText(title, Qt::ElideMiddle, std::max(0, label_->width() - 4)));
}

}
