#include "debugPanel.h"
#include "debugOpenGlWidget.h"
#include <QVBoxLayout>
#include <QApplication>


namespace MOON {

	class DebugWidget::DebugWidgetInternal {
	public:
		DebugWidgetInternal(DebugWidget* debugWidget) :self(debugWidget) {

		}
	private:
		DebugWidget* self = nullptr;
	};
	DebugWidget::DebugWidget(QWidget* parent) :QDockWidget(parent), mInternal(new DebugWidgetInternal(this))
	{
		QWidget* center = new QWidget(this);
		this->setWidget(center);
		QVBoxLayout* layout = new QVBoxLayout(center);
		layout->addWidget(new DebugOpenGLWidget(center));
		this->setWindowTitle(QApplication::translate("DebugWdiget", "Debug", nullptr));

	}
	DebugWidget::~DebugWidget()
	{
		delete mInternal;
	}
}



