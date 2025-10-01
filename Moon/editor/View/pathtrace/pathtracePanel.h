#pragma once
#include <QWidget>
namespace MOON {
	class PathTracePanel : public QWidget
	{
	public:
		explicit PathTracePanel(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
	};
}