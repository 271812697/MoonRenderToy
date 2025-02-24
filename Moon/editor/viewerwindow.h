#pragma once
#include "viewer/Viewer.h"
#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_5_Core>
#include <QtNodes/NodeData>
using QtNodes::NodeData;
namespace MOON {

	class Editor;
	class ViewerWindow : public QOpenGLWidget, QOpenGLFunctions_4_5_Core
	{
		Q_OBJECT
	public:
		explicit ViewerWindow(QWidget* parent);
		~ViewerWindow();


		virtual void initializeGL() override;
		virtual void timerEvent(QTimerEvent* e) override;
		virtual void paintGL() override;

		virtual void leaveEvent(QEvent* event) override;

		virtual void resizeEvent(QResizeEvent* event) override;

		virtual void mousePressEvent(QMouseEvent* event) override;

		virtual void mouseMoveEvent(QMouseEvent* event) override;

		virtual void mouseReleaseEvent(QMouseEvent* event) override;

		virtual void wheelEvent(QWheelEvent* event) override;
		virtual void keyPressEvent(QKeyEvent* event) override;
		virtual void keyReleaseEvent(QKeyEvent* event) override;
	public:
		Viewer viewer;
	private:
		bool blockMouseMessage = false;

	public Q_SLOTS:
		void viewnode(const std::shared_ptr<NodeData>& node);
	};
}