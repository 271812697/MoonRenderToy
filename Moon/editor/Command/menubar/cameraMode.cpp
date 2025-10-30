#include "cameraMode.h"

#include "OvCore/Global/ServiceLocator.h"
#include "core/log.h"
#include "core/ServiceLocator.h"
#include "renderer/SceneView.h"

#include <QMenu>
#include <QtWidgets/QFileDialog>
#include <QCoreApplication>


namespace MOON {
	//-----------------------------------------------------------------------------
	CameraModeComand::CameraModeComand(QObject* parentObject)
		: Command(parentObject)
	{
		mActionOrtho = new QAction(Command::tr("Orthographic"), this);
		mActionPersp = new QAction(Command::tr("Perspective"), this);
		mActionOrtho->setCheckable(true);
		mActionPersp->setCheckable(true);
		auto menu = new QMenu(static_cast<QWidget*>(parentObject));
		menu->addAction(mActionOrtho);
		menu->addAction(mActionPersp);
		auto group = new QActionGroup(menu);
		group->setExclusive(true);
		group->addAction(mActionOrtho);
		group->addAction(mActionPersp);

		auto action = new QAction(this);
		action->setText(Command::tr("Projection"));
		action->setMenu(menu);
		setAction(action);
		setIcon(":/widgets/icons/pqCamera.svg");
		
		action->setObjectName(QString::fromUtf8("actionCameraMode"));
		action->setText("&Camera Mode");
		action->setStatusTip("Switch to Camera Mode");
		action->setShortcut(QCoreApplication::translate("pqFileMenuBuilder", "Ctrl+M", nullptr));

		QObject::connect(group, &QActionGroup::triggered, this, [=](const QAction* action) {
			auto& view=GetService(OvEditor::Panels::SceneView);
			if (action == mActionOrtho) {
				view.setCameraMode(OvRendering::Settings::EProjectionMode::ORTHOGRAPHIC);
				const auto& forward=view.GetCamera()->GetTransform().GetWorldForward();
				view.FitToSelectedActor({1,0,0});
			}
			else
			{
				view.setCameraMode(OvRendering::Settings::EProjectionMode::PERSPECTIVE);
			}

			});

	}

	void CameraModeComand::execute()
	{
		CORE_INFO("Switch to Camera Mode");
		

	}


}




