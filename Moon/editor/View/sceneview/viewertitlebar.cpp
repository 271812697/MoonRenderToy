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
				auto matList=view.GetSelectedActor().GetComponent<Core::ECS::Components::CMaterialRenderer>();
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
				auto matList = view.GetSelectedActor().GetComponent<Core::ECS::Components::CMaterialRenderer>();
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
			QHBoxLayout* layout = new QHBoxLayout(mSelf);
			layout->setSpacing(0);
			layout->setContentsMargins(0, 0, 0, 0);
			//auto horizontalSpacer = new QSpacerItem(40, 14, QSizePolicy::Expanding, QSizePolicy::Minimum);
			//layout->addItem(horizontalSpacer);
			mToolBar = new QToolBar(mSelf);
			mToolBar->setIconSize(QSize(30, 30));
			mToolBar->layout()->setSpacing(0);
			mToolBar->layout()->setContentsMargins(0, 0, 0, 0);
			//layout->insertWidget(0,mToolBar);
			layout->addWidget(mToolBar);


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
			mToolBar->addAction(xPlus->action());
			
			xMinus->setIcon(QString::fromUtf8(":/widgets/icons/pqXMinus.svg"));
			mToolBar->addAction(xMinus->action());
			yPlus->setIcon(QString::fromUtf8(":/widgets/icons/pqYPlus.svg"));
			mToolBar->addAction(yPlus->action());
			yMinus->setIcon(QString::fromUtf8(":/widgets/icons/pqYMinus.svg"));
			mToolBar->addAction(yMinus->action());
			zPlus->setIcon(QString::fromUtf8(":/widgets/icons/pqZPlus.svg"));
			mToolBar->addAction(zPlus->action());
			zMinus->setIcon(QString::fromUtf8(":/widgets/icons/pqZMinus.svg"));
			mToolBar->addAction(zMinus->action());
			isometricView->setIcon(QString::fromUtf8(":/widgets/icons/pqIsometricView.svg"));
			mToolBar->addAction(isometricView->action());
			zoomToSelection->setIcon(QString::fromUtf8(":/widgets/icons/pqZoomToSelection.svg"));
			mToolBar->addAction(zoomToSelection->action());
			rotateCameraCW->setIcon(QString::fromUtf8(":/widgets/icons/pqRotateCameraCW.svg"));
			mToolBar->addAction(rotateCameraCW->action());
			rotateCameraCCW->setIcon(QString::fromUtf8(":/widgets/icons/pqRotateCameraCCW.svg"));
			mToolBar->addAction(rotateCameraCCW->action());

			wire = new WireCommand(mSelf);
			mToolBar->addAction(wire->action());

			points = new PointsCommand(mSelf);
			mToolBar->addAction(points->action());

			measure = new MeasureCommand(mSelf);
			mToolBar->addAction(measure->action());
			clip = new ClipCommand(mSelf);
			mToolBar->addAction(clip->action());
		
		}
		~ViewerWindowTitleBarInternal() {

		}
	private:
		QPointer<QToolBar> mToolBar;
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
	ViewerWindowTitleBar::ViewerWindowTitleBar(QWidget* parent) :QWidget(parent), mInternal(new ViewerWindowTitleBarInternal(this))
	{


	}
	ViewerWindowTitleBar::~ViewerWindowTitleBar()
	{
		delete mInternal;
	}
}