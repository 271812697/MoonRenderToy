#pragma once
#include "core/logOutput.h"
#include <QDockWidget>
#include <QString>
#include <deque>
#include <string>
#include <utility>

class QCheckBox;
class QPlainTextEdit;
class QPushButton;

namespace MOON {

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
		void rebuildView();
		void showMenu(const QPoint& point);
		void copyLogContent();

	private:
		void appendMessage(int level, const QString& msg);
		bool levelVisible(int level) const;

		QPlainTextEdit* m_logText = nullptr;
		QCheckBox* m_debugCheck = nullptr;
		QCheckBox* m_infoCheck = nullptr;
		QCheckBox* m_errorCheck = nullptr;
		QCheckBox* m_warnCheck = nullptr;
		QPushButton* m_clear = nullptr;

		std::deque<std::pair<int, QString>> m_messages;
	};

}
