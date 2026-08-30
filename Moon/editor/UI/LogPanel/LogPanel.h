#pragma once
#include "core/logOutput.h"
#include <QDockWidget>
#include <QString>

class QCheckBox;
class QPushButton;
class QTableView;

namespace MOON {

	class LogTableModel;

	// Console-style log panel with Level / Time / Message columns. Rows live in
	// a table model (not one widget per line), so high message rates stay cheap.
	class LogPanel : public QDockWidget, public LogOutput {
		Q_OBJECT
	public:
		LogPanel(QWidget* parent = nullptr);
		~LogPanel();

		// LogOutput
		virtual void logMessage(Level level, const std::string& msg) override;

	signals:
		void postMessage(int level, QString);

	private slots:
		void onLogMessage(int level, QString msg);
		void onClearMessage();
		void showMenu(const QPoint& point);
		void copyLogContent();

	private:
		QTableView* m_logView = nullptr;
		LogTableModel* m_model = nullptr;
		QCheckBox* m_debugCheck = nullptr;
		QCheckBox* m_infoCheck = nullptr;
		QCheckBox* m_errorCheck = nullptr;
		QCheckBox* m_warnCheck = nullptr;
		QPushButton* m_clear = nullptr;
	};

}
