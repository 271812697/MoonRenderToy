#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_5_Core>
namespace OvCore::ECS {
	class Actor;
}
namespace MOON {

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

	public slots:
		void onActorSelected(OvCore::ECS::Actor* actor);
		void onSceneChange(const QString& path);
	signals:
		void sceneChange();
		
	private:
		class ViewerWindowInternal;
		ViewerWindowInternal* mInternal = nullptr;


	};
}