#include "TopoShape.h"
#include <gp_Pln.hxx>
#include "editor/Command/menubar/sketch.h"
#include "editor/View/sceneview/viewerwidget.h"
#include "editor/UI/TaskPanel/TaskViewWidget.h"
#include "editor/UI/TaskPanel/SketchTaskDialog.h"
#include "renderer/SceneView.h"
#include "renderer/GizmoRenderPass.h"
#include "Core/Global/ServiceLocator.h"
#include "core/log.h"
#include "core/SelectionManager.h"
#include "core/component/CTopoShape.h"
#include "Sketcher/SketcherObjManager.h"
#include "Sketcher/SketcherObj.h"
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
		auto& task = GetService(TaskViewWidget);
		if (!task.hasTask()) {
			SketchTaskDialog* sketchTask=new SketchTaskDialog();
			task.setTaskDialog(sketchTask);
			std::vector<SelectID> selectIds = SelectionManager::instance().getSelect();
			bool enableSelectXYZPlane = true;
			if (!selectIds.empty()) {
				auto& view = GetService(Editor::Panels::SceneView);
				auto scene = view.GetScene();
				auto actor = scene->FindActorByID(selectIds.back());
				if (actor->HasParent()) {
					auto parent = actor->GetParent();
					if (parent->HasParent()) {
						auto grandParent = parent->GetParent();
						if (grandParent->HasComponent("CTopoShape")) {
							auto topoComp = grandParent->GetComponent<::Core::ECS::Components::CTopoShape>();
							int childId = parent->GetChildId(actor);
							if (parent->HasComponent("CBatchMeshTriangle")) {
								if (childId != -1) {
									Part::TopoShape shape = topoComp->GetTopoFace(childId);
									gp_Pln pln;
									shape.findPlane(pln);
									auto sketchObj = SketcherObjManager::instance().GetCurrentActiveSketcherObj();
									SketcherPlane2D plane;
									plane.normal = Base::Vector3d{ pln.Axis().Direction().X(),pln.Axis().Direction().Y(),pln.Axis().Direction().Z() };
									plane.origin = Base::Vector3d{ pln.Location().X(),pln.Location().Y(),pln.Location().Z() };
									plane.xAxis = Base::Vector3d{ pln.XAxis().Direction().X(),pln.XAxis().Direction().Y(),pln.XAxis().Direction().Z() };
									plane.yAxis = Base::Vector3d{ pln.YAxis().Direction().X(),pln.YAxis().Direction().Y(),pln.YAxis().Direction().Z() };
									sketchObj->setPlane(plane);
									sketchObj->setBasedTopoShape(topoComp->GetTopoShape());
									enableSelectXYZPlane = false;
								}
							}
							else if (parent->HasComponent("CBatchMeshLine")) {
								if (childId != -1) {
									//topoComp->hoverChildLine(childId);
									//topoComp->hoverChild(childId);
								}
							}
						}
					}
				}
			}
			if(enableSelectXYZPlane)
			{
				auto& view = GetService(Editor::Panels::SceneView);
				auto& gizmoPass = view.GetRenderer().GetPass<Editor::Rendering::GizmoRenderPass>("ImRenderer");
				gizmoPass.enableGizmoWidget("GizmoSketchPlane", true);
			}
		}
	}
}




