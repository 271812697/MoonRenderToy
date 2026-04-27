#pragma once
#include <QToolBar>
namespace MOON {
	class PrimitiveToolbar : public QToolBar
	{
		Q_OBJECT
	public:
		PrimitiveToolbar(const QString& title, QWidget* parent = nullptr);
		PrimitiveToolbar(QWidget* parentObject = nullptr);
		~PrimitiveToolbar()override;
	private:
		class PrimitiveToolbarInternal;
		PrimitiveToolbarInternal* mInternal = nullptr;
		void constructor();
	};
}