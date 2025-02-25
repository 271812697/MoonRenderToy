#pragma once
#include "viewer/Viewer.h"
#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_5_Core>
#include <QtNodes/NodeData>
using QtNodes::NodeData;
namespace MOON {
	class vtkRenderWindowInteractor;
	class vtkBoxWidget2;
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
		bool processEventByWindowInteractor(QEvent* event);
		bool blockMouseMessage = false;
		vtkRenderWindowInteractor* windowInteractor=nullptr;
		vtkBoxWidget2* boxWidget = nullptr;

	public Q_SLOTS:
		void viewnode(const std::shared_ptr<NodeData>& node);
	};
}