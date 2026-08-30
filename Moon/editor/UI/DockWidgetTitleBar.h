#pragma once
#include <QWidget>

class QDockWidget;
class QLabel;
class QToolButton;

namespace MOON {

// Inviwo-style dock widget title bar: a solid header with an elided title
// label and flat float/close buttons on the right.
class DockWidgetTitleBar : public QWidget {
    Q_OBJECT
public:
    explicit DockWidgetTitleBar(QDockWidget* parent);
    ~DockWidgetTitleBar();

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void updateTitle();

    QDockWidget* dock_ = nullptr;
    QLabel* label_ = nullptr;
    QToolButton* floatBtn_ = nullptr;
    QToolButton* closeBtn_ = nullptr;
};

}
