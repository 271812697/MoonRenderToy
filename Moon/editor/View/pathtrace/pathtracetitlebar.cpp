#include "pathtracetitlebar.h"
#include "editor/Command/pathtrace/CameraReaction.h"
#include <QHBoxLayout>
#include <QToolBar>
#include <QPointer>

namespace MOON {
	class PathTraceTitleBar::PathTraceTitleBarInternal {
	public:
		PathTraceTitleBarInternal(PathTraceTitleBar* titleBar) :mSelf(titleBar) {
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
			buildReaction();

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
		}
		void buildReaction() {
			xMinus = new CameraReaction(mSelf, CameraReaction::Mode::RESET_NEGATIVE_X);
			xPlus = new CameraReaction(mSelf, CameraReaction::Mode::RESET_POSITIVE_X);
			yMinus = new CameraReaction(mSelf, CameraReaction::Mode::RESET_NEGATIVE_Y);
			yPlus = new CameraReaction(mSelf, CameraReaction::Mode::RESET_POSITIVE_Y);
			zMinus = new CameraReaction(mSelf, CameraReaction::Mode::RESET_NEGATIVE_Z);
			zPlus = new CameraReaction(mSelf, CameraReaction::Mode::RESET_POSITIVE_Z);
			isometricView = new CameraReaction(mSelf, CameraReaction::Mode::APPLY_ISOMETRIC_VIEW);
			zoomToSelection = new CameraReaction(mSelf, CameraReaction::Mode::ZOOM_TO_DATA);
			rotateCameraCCW = new CameraReaction(mSelf, CameraReaction::Mode::ROTATE_CAMERA_CCW);
			rotateCameraCW = new CameraReaction(mSelf, CameraReaction::Mode::ROTATE_CAMERA_CW);

		}
		~PathTraceTitleBarInternal() {

		}
	private:
		QPointer<QToolBar> mToolBar;
		PathTraceTitleBar* mSelf = nullptr;
		CameraReaction* xMinus = nullptr;
		CameraReaction* xPlus = nullptr;
		CameraReaction* yMinus = nullptr;
		CameraReaction* yPlus = nullptr;
		CameraReaction* zMinus = nullptr;
		CameraReaction* zPlus = nullptr;
		CameraReaction* isometricView = nullptr;
		CameraReaction* zoomToSelection = nullptr;
		CameraReaction* rotateCameraCCW = nullptr;
		CameraReaction* rotateCameraCW = nullptr;
	};
	PathTraceTitleBar::PathTraceTitleBar(QWidget* parent) :QWidget(parent), mInternal(new PathTraceTitleBarInternal(this))
	{


	}
	PathTraceTitleBar::~PathTraceTitleBar()
	{
		delete mInternal;
	}
}