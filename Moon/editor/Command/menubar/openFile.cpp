#include "openFile.h"
#include "editor/View/sceneview/viewerwidget.h"
#include "editor/View/pathtrace/pathtraceWidget.h"
#include "Core/Global/ServiceLocator.h"
#include "core/log.h"
#include <QtWidgets/QFileDialog>
#include <QCoreApplication>
namespace MOON {
	//-----------------------------------------------------------------------------
	OpenFileCommand::OpenFileCommand(QObject* parentObject)
		: Command(parentObject)
	{
		
		auto& viewer = GetService(ViewerWidget);
		auto& pathTrace = GetService(PathTraceWidget);
		connect(this, &OpenFileCommand::sceneChange, &viewer, &ViewerWidget::onSceneChange);
		connect(this, &OpenFileCommand::sceneChange, &pathTrace, &PathTraceWidget::onSceneChange);
		auto openfile=new QAction(this);
		setAction(openfile);
		openfile->setObjectName(QString::fromUtf8("actionFileOpen"));
		openfile->setText("&Open");
		openfile->setStatusTip("Open");
		openfile->setShortcut(QCoreApplication::translate("pqFileMenuBuilder", "Ctrl+O", nullptr));
		QIcon icon9;
		icon9.addFile(QString::fromUtf8(":/widgets/icons/pqOpen.svg"), QSize(), QIcon::Normal, QIcon::Off);
		openfile->setIcon(icon9);
	}

	void OpenFileCommand::execute()
	{
		/*
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

		*/
		QString fileName = QFileDialog::getOpenFileName(nullptr,
			tr("Open Flow Scene"),
			QDir::homePath(),
			tr("Flow Scene Files (*.scene;*.gltf;*.obj;*.stl;*.*)"));
		if (!QFileInfo::exists(fileName))
			return;
		CORE_INFO("Switch to Scene {0}", fileName.toStdString());
		emit sceneChange(fileName);
	}


}




