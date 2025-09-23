#pragma once
#include <QObject>
namespace MOON {
	class ParseScene :public QObject {
		Q_OBJECT
	public:
		ParseScene(QObject* parent);
		void ParsePathTraceScene();
	public slots:
		void updateTreeViewSceneRoot();
		void updateTreeViewPathRoot();



	};

}