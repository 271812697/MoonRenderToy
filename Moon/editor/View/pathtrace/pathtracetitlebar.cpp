#include "pathtracetitlebar.h"
#include "editor/Reaction/pathtrace/CameraReaction.h"
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


			xPlus = new QAction(mToolBar);
			xPlus->setEnabled(true);
			QIcon icon1;
			icon1.addFile(QString::fromUtf8(":/widgets/icons/pqXPlus.svg"), QSize(), QIcon::Normal, QIcon::On);
			xPlus->setIcon(icon1);
			mToolBar->addAction(xPlus);
			xMinus = new QAction(mToolBar);
			xMinus->setEnabled(true);
			QIcon icon2;
			icon2.addFile(QString::fromUtf8(":/widgets/icons/pqXMinus.svg"), QSize(), QIcon::Normal, QIcon::On);
			xMinus->setIcon(icon2);
			mToolBar->addAction(xMinus);

			yPlus = new QAction(mToolBar);
			yPlus->setEnabled(true);
			QIcon icon3;
			icon3.addFile(QString::fromUtf8(":/widgets/icons/pqYPlus.svg"), QSize(), QIcon::Normal, QIcon::On);
			yPlus->setIcon(icon3);
			mToolBar->addAction(yPlus);
			yMinus = new QAction(mToolBar);
			yMinus->setEnabled(true);
			QIcon icon4;
			icon4.addFile(QString::fromUtf8(":/widgets/icons/pqYMinus.svg"), QSize(), QIcon::Normal, QIcon::On);
			yMinus->setIcon(icon4);
			mToolBar->addAction(yMinus);

			zPlus = new QAction(mToolBar);
			zPlus->setEnabled(true);
			QIcon icon5;
			icon5.addFile(QString::fromUtf8(":/widgets/icons/pqZPlus.svg"), QSize(), QIcon::Normal, QIcon::On);
			zPlus->setIcon(icon5);
			mToolBar->addAction(zPlus);
			zMinus = new QAction(mToolBar);
			zMinus->setEnabled(true);
			QIcon icon6;
			icon6.addFile(QString::fromUtf8(":/widgets/icons/pqZMinus.svg"), QSize(), QIcon::Normal, QIcon::On);
			zMinus->setIcon(icon6);
			mToolBar->addAction(zMinus);

			isometricView = new QAction(mToolBar);
			isometricView->setEnabled(true);
			QIcon icon7;
			icon7.addFile(QString::fromUtf8(":/widgets/icons/pqIsometricView.svg"), QSize(), QIcon::Normal, QIcon::On);
			isometricView->setIcon(icon7);
			mToolBar->addAction(isometricView);

			zoomToSelection = new QAction(mToolBar);
			zoomToSelection->setEnabled(true);
			QIcon icon8;
			icon8.addFile(QString::fromUtf8(":/widgets/icons/pqZoomToSelection.svg"), QSize(), QIcon::Normal, QIcon::On);
			zoomToSelection->setIcon(icon8);
			mToolBar->addAction(zoomToSelection);

			rotateCameraCW = new QAction(mToolBar);
			rotateCameraCW->setEnabled(true);
			QIcon icon9;
			icon9.addFile(QString::fromUtf8(":/widgets/icons/pqRotateCameraCW.svg"), QSize(), QIcon::Normal, QIcon::On);
			rotateCameraCW->setIcon(icon9);
			mToolBar->addAction(rotateCameraCW);

			rotateCameraCCW = new QAction(mToolBar);
			rotateCameraCCW->setEnabled(true);
			QIcon icon10;
			icon10.addFile(QString::fromUtf8(":/widgets/icons/pqRotateCameraCCW.svg"), QSize(), QIcon::Normal, QIcon::On);
			rotateCameraCCW->setIcon(icon10);
			mToolBar->addAction(rotateCameraCCW);

			buildReaction();
		}
		void buildReaction() {
			new CameraReaction(xMinus, CameraReaction::Mode::RESET_NEGATIVE_X);
			new CameraReaction(yMinus, CameraReaction::Mode::RESET_NEGATIVE_Y);
			new CameraReaction(zMinus, CameraReaction::Mode::RESET_NEGATIVE_Z);
			new CameraReaction(xPlus, CameraReaction::Mode::RESET_POSITIVE_X);
			new CameraReaction(yPlus, CameraReaction::Mode::RESET_POSITIVE_Y);
			new CameraReaction(zPlus, CameraReaction::Mode::RESET_POSITIVE_Z);
			new CameraReaction(isometricView, CameraReaction::Mode::APPLY_ISOMETRIC_VIEW);
			new CameraReaction(zoomToSelection, CameraReaction::Mode::ZOOM_TO_DATA);
			new CameraReaction(rotateCameraCCW, CameraReaction::Mode::ROTATE_CAMERA_CCW);
			new CameraReaction(rotateCameraCW, CameraReaction::Mode::ROTATE_CAMERA_CW);

		}
		~PathTraceTitleBarInternal() {

		}
	private:
		QPointer<QToolBar> mToolBar;
		PathTraceTitleBar* mSelf = nullptr;
		QAction* xMinus = nullptr;
		QAction* xPlus = nullptr;
		QAction* yMinus = nullptr;
		QAction* yPlus = nullptr;
		QAction* zMinus = nullptr;
		QAction* zPlus = nullptr;
		QAction* isometricView = nullptr;
		QAction* zoomToSelection = nullptr;
		QAction* rotateCameraCCW = nullptr;
		QAction* rotateCameraCW = nullptr;
	};
	PathTraceTitleBar::PathTraceTitleBar(QWidget* parent) :QWidget(parent), mInternal(new PathTraceTitleBarInternal(this))
	{


	}
	PathTraceTitleBar::~PathTraceTitleBar()
	{
		delete mInternal;
	}
}