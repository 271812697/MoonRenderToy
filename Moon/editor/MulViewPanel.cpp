#include "MulViewPanel.h"
#include "editor/View/sceneview/viewerpanel.h"
#include <QTabWidget>
#include <QGridLayout>
namespace MOON {

	class MulViewPanel::MulViewPanelImpl {
	public:
		QTabWidget* tabWidget = nullptr;

	};


	MulViewPanel::MulViewPanel(QWidget* parent) :QWidget(parent)
	{
		impl = new MulViewPanelImpl();
		impl->tabWidget = new QTabWidget(this);
		QGridLayout* glayout = new QGridLayout(this);
		glayout->setContentsMargins(0, 0, 0, 0);
		glayout->setSpacing(0);
		glayout->addWidget(impl->tabWidget, 0, 0);
		
		impl->tabWidget->addTab(new ViewerPanel(this), "viewerpanel");

	}

	MulViewPanel::~MulViewPanel()
	{
		delete impl;
	}

}