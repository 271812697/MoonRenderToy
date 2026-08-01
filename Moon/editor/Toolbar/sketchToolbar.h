#pragma once
#include <QToolBar>
namespace MOON {
	class SketchToolbar : public QToolBar
	{
		Q_OBJECT
	public:
		SketchToolbar(const QString& title, QWidget* parent = nullptr);
		SketchToolbar(QWidget* parentObject = nullptr);
		~SketchToolbar()override;
		void disableAllHandlers();
		void setUncheckedAction(const std::string&name);
	private:
		class SketchToolbarInternal;
		SketchToolbarInternal* mInternal = nullptr;
		void constructor();
	};
}