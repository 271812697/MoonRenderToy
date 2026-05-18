#pragma once
#include <QToolBar>
namespace MOON {
	class DesignModelingToolbar : public QToolBar
	{
		Q_OBJECT
	public:
		DesignModelingToolbar(const QString& title, QWidget* parent = nullptr);
		DesignModelingToolbar(QWidget* parentObject = nullptr);
		~DesignModelingToolbar()override;
	private:
		class DesignModelingToolbarInternal;
		DesignModelingToolbarInternal* mInternal = nullptr;
		void constructor();
	};
}