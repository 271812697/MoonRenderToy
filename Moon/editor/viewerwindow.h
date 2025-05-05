#pragma once
#include "viewer/Viewer.h"
#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_5_Core>
#include <QtNodes/NodeData>
using QtNodes::NodeData;
namespace MOON {
	class RenderWindowInteractor;
	class BoxWidget2;
	class Editor;
	class ViewerWindow : public QOpenGLWidget, QOpenGLFunctions_4_5_Core
	{
		Q_OBJECT
	public:
		explicit ViewerWindow(QWidget* parent);
		~ViewerWindow();
		void initializeGL() override;
		void timerEvent(QTimerEvent* e) override;
		void paintGL() override;
		bool event(QEvent* evt) override;
		void leaveEvent(QEvent* event) override;

		void resizeEvent(QResizeEvent* event) override;

		void mousePressEvent(QMouseEvent* event) override;

		void mouseMoveEvent(QMouseEvent* event) override;

		void mouseReleaseEvent(QMouseEvent* event) override;

		void wheelEvent(QWheelEvent* event) override;
		void keyPressEvent(QKeyEvent* event) override;
		void keyReleaseEvent(QKeyEvent* event) override;
	public:
		Viewer viewer;
	private:

		bool blockMouseMessage = false;
		RenderWindowInteractor* windowInteractor = nullptr;
		BoxWidget2* boxWidget = nullptr;
		bool initFlag = false;


	public Q_SLOTS:
		void viewnode(const std::shared_ptr<NodeData>& node);
	};
}