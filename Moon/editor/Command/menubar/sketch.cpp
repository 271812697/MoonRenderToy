#include "editor/Command/menubar/sketch.h"
#include "editor/View/sceneview/viewerwidget.h"
#include "editor/UI/TaskPanel/TaskViewWidget.h"
#include "editor/UI/TaskPanel/SketchTaskDialog.h"
#include "renderer/SceneView.h"
#include "renderer/GizmoRenderPass.h"
#include "Core/Global/ServiceLocator.h"
#include "core/log.h"
#include <QtWidgets/QFileDialog>
#include <QCoreApplication>
namespace MOON {
	//-----------------------------------------------------------------------------
	SketchCommand::SketchCommand(QObject* parentObject)
		: Command(parentObject)
	{
		
		
		
		
		auto sketch=new QAction(this);
		sketch->setCheckable(true);
		setAction(sketch);
		sketch->setObjectName(QString::fromUtf8("createsketch"));
		sketch->setText("&create sketch");
		sketch->setStatusTip("create sketch");
		//sketch->setShortcut(QCoreApplication::translate("pqFileMenuBuilder", "Ctrl+O", nullptr));
		QIcon icon9;
		icon9.addFile(QString::fromUtf8(":/widgets/icons/Sketcher_NewSketch.svg"), QSize(), QIcon::Normal, QIcon::Off);
		sketch->setIcon(icon9);
	}

	void SketchCommand::execute()
	{
		bool value = action()->isChecked();
		auto& view = GetService(Editor::Panels::SceneView);
		auto& gizmoPass = view.GetRenderer().GetPass<Editor::Rendering::GizmoRenderPass>("Gizmo");
		gizmoPass.enableGizmoWidget(
			"GizmoSketchPlane", value);
		auto& task = GetService(TaskViewWidget);
		if(value)
		task.setTaskDialog(new SketchTaskDialog());
		else
		{
			task.clearTask();
		}
	}
}




