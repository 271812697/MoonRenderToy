#pragma once
#include <QObject>
#include <string>
namespace MOON {
	class ParseScene :public QObject {
		Q_OBJECT
	public:
		ParseScene(QObject* parent);
		void ParsePathTraceScene(const std::string& path);
	public slots:
		void updateTreeViewSceneRoot();
		void updateTreeViewPathRoot();
	};

}