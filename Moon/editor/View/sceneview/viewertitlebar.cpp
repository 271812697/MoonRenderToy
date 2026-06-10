#include "viewertitlebar.h"
#include "Core/Global/ServiceLocator.h"
#include "renderer/SceneView.h"
#include "Core/ECS/Components/CMaterialRenderer.h"
#include "editor/Command/viewer/CameraFitCommand.h"
#include "renderer/PointRenderPass.h"
#include "renderer/GizmoRenderPass.h"
#include "core/callbackManager.h"
#include <QHBoxLayout>
#include <QToolBar>
#include <QPointer>

namespace MOON {

	class  WireCommand : public Command
	{
	public:
		WireCommand(QObject* parent) :Command(parent) {
			auto action = new QAction(this);
			action->setCheckable(true);
		
			setAction(action);
			setIcon(QString::fromUtf8(":/widgets/icons/wire.png"));
		}
	protected:
		virtual void execute()override {
			bool value=action()->isChecked();
			auto& view = GetService(Editor::Panels::SceneView);
			if (view.IsSelectActor()) {
				auto matList=view.GetSelectedActor()->GetComponent<Core::ECS::Components::CMaterialRenderer>();
				if (matList) {
					auto mat = matList->GetMaterialAtIndex(0);
					if (mat&&mat->SupportsFeature("WITH_EDGE")) {
						mat->EnableFeature("WITH_EDGE",value);
					}
				}
			}
		}
	};
	class  PointsCommand : public Command
	{
	public:
		PointsCommand(QObject* parent) :Command(parent) {
			auto action = new QAction(this);
			action->setCheckable(true);
			setAction(action);
			setIcon(QString::fromUtf8(":/widgets/icons/points.png"));
			
		}
	protected:
		virtual void execute()override {
			bool value = action()->isChecked();
			auto& view = GetService(Editor::Panels::SceneView);
			view.GetRenderer().GetPass<Editor::Rendering::PointRenderPass>("PointDraw").SetEnabled(value);
		}
	};
	class  MeasureCommand : public Command
	{
	public:
		MeasureCommand(QObject* parent) :Command(parent) {
			auto action = new QAction(this);
			action->setCheckable(true);
			setAction(action);
			setIcon(QString::fromUtf8(":/widgets/icons/pqRuler.svg"));
			createCallBack(CallBackManager::instance(), [this]() {
				this->execute();
				});

		}
	protected:
		virtual void execute()override {
			bool value = action()->isChecked();
			auto& view = GetService(Editor::Panels::SceneView);
			view.GetRenderer().GetPass<Editor::Rendering::GizmoRenderPass>("Gizmo").enableGizmoWidget("Measure",value);
		}
	};
	class  ClipCommand : public Command
	{
	public:
		ClipCommand(QObject* parent) :Command(parent) {
			auto action = new QAction(this);
			action->setCheckable(true);
			setAction(action);
			setIcon(QString::fromUtf8(":/widgets/icons/pqClip.svg"));
			//createCallBack(CallBackManager::instance(), [this]() {
			//	this->execute();
			//	});


		}
	protected:
		virtual void execute()override {
			bool value = action()->isChecked();
			auto& view = GetService(Editor::Panels::SceneView);
			view.GetRenderer().GetPass<Editor::Rendering::GizmoRenderPass>("Gizmo").enableGizmoWidget("ClipPlane", value);
			
			if (view.IsSelectActor()) {
				auto& selectActor=*view.GetSelectedActor();
				auto matList = selectActor.GetComponent<Core::ECS::Components::CMaterialRenderer>();
				if (matList) {
					auto mat = matList->GetMaterialAtIndex(0);
					if (mat && mat->SupportsFeature("CLIP_PLANE")) {
						mat->EnableFeature("CLIP_PLANE", value);
					}
				}
			
			}
		}
	};
	class ViewerWindowTitleBar::ViewerWindowTitleBarInternal {
	public:
		ViewerWindowTitleBarInternal(ViewerWindowTitleBar* titleBar) :mSelf(titleBar) {



			xMinus = new CameraFitCommand(mSelf, CameraFitCommand::Mode::RESET_NEGATIVE_X);
			xPlus = new CameraFitCommand(mSelf, CameraFitCommand::Mode::RESET_POSITIVE_X);
			yMinus = new CameraFitCommand(mSelf, CameraFitCommand::Mode::RESET_NEGATIVE_Y);
			yPlus = new CameraFitCommand(mSelf, CameraFitCommand::Mode::RESET_POSITIVE_Y);
			zMinus = new CameraFitCommand(mSelf, CameraFitCommand::Mode::RESET_NEGATIVE_Z);
			zPlus = new CameraFitCommand(mSelf, CameraFitCommand::Mode::RESET_POSITIVE_Z);
			isometricView = new CameraFitCommand(mSelf, CameraFitCommand::Mode::APPLY_ISOMETRIC_VIEW);
			zoomToSelection = new CameraFitCommand(mSelf, CameraFitCommand::Mode::ZOOM_TO_DATA);
			rotateCameraCCW = new CameraFitCommand(mSelf, CameraFitCommand::Mode::ROTATE_CAMERA_CCW);
			rotateCameraCW = new CameraFitCommand(mSelf, CameraFitCommand::Mode::ROTATE_CAMERA_CW);

			xPlus->setIcon(QString::fromUtf8(":/widgets/icons/pqXPlus.svg"));
			mSelf->addAction(xPlus->action());
			
			xMinus->setIcon(QString::fromUtf8(":/widgets/icons/pqXMinus.svg"));
			mSelf->addAction(xMinus->action());
			yPlus->setIcon(QString::fromUtf8(":/widgets/icons/pqYPlus.svg"));
			mSelf->addAction(yPlus->action());
			yMinus->setIcon(QString::fromUtf8(":/widgets/icons/pqYMinus.svg"));
			mSelf->addAction(yMinus->action());
			zPlus->setIcon(QString::fromUtf8(":/widgets/icons/pqZPlus.svg"));
			mSelf->addAction(zPlus->action());
			zMinus->setIcon(QString::fromUtf8(":/widgets/icons/pqZMinus.svg"));
			mSelf->addAction(zMinus->action());
			isometricView->setIcon(QString::fromUtf8(":/widgets/icons/pqIsometricView.svg"));
			mSelf->addAction(isometricView->action());
			zoomToSelection->setIcon(QString::fromUtf8(":/widgets/icons/pqZoomToSelection.svg"));
			mSelf->addAction(zoomToSelection->action());
			rotateCameraCW->setIcon(QString::fromUtf8(":/widgets/icons/pqRotateCameraCW.svg"));
			mSelf->addAction(rotateCameraCW->action());
			rotateCameraCCW->setIcon(QString::fromUtf8(":/widgets/icons/pqRotateCameraCCW.svg"));
			mSelf->addAction(rotateCameraCCW->action());

			wire = new WireCommand(mSelf);
			mSelf->addAction(wire->action());

			points = new PointsCommand(mSelf);
			mSelf->addAction(points->action());

			measure = new MeasureCommand(mSelf);
			mSelf->addAction(measure->action());
			clip = new ClipCommand(mSelf);
			mSelf->addAction(clip->action());
		
		}
		~ViewerWindowTitleBarInternal() {

		}
	private:
		
		ViewerWindowTitleBar* mSelf = nullptr;
		
		CameraFitCommand* xMinus = nullptr;
		CameraFitCommand* xPlus = nullptr;
		CameraFitCommand* yMinus = nullptr;
		CameraFitCommand* yPlus = nullptr;
		CameraFitCommand* zMinus = nullptr;
		CameraFitCommand* zPlus = nullptr;
		CameraFitCommand* isometricView = nullptr;
		CameraFitCommand* zoomToSelection = nullptr;
		CameraFitCommand* rotateCameraCCW = nullptr;
		CameraFitCommand* rotateCameraCW = nullptr;
		WireCommand* wire = nullptr;
		PointsCommand* points = nullptr;
		MeasureCommand* measure = nullptr;
		ClipCommand* clip = nullptr;
	};
	ViewerWindowTitleBar::ViewerWindowTitleBar(QWidget* parent) :QToolBar(parent), mInternal(new ViewerWindowTitleBarInternal(this))
	{


	}
	ViewerWindowTitleBar::~ViewerWindowTitleBar()
	{
		delete mInternal;
	}
}