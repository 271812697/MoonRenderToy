#pragma once
#include <QWidget>
namespace MOON {


	class PathTraceTitleBar : public QWidget
	{
	public:
		explicit PathTraceTitleBar(QWidget* parent = nullptr);
		~PathTraceTitleBar();
	private:
		class PathTraceTitleBarInternal;
		PathTraceTitleBarInternal* mInternal = nullptr;
	};
}