#pragma once
#include "UILogPanel.h"
#include "core/logOutput.h"
#include <QDockWidget>
#include <QString>
#include <string>

namespace MOON
{

	class LogPanel : public QDockWidget,public LogOutput, public Ui_LogPanel
	{
		Q_OBJECT

	public:
		LogPanel(QWidget* parent = 0);
		~LogPanel();

		// Out
		void OutMsg( int level,const char* msg, const char* icon);

	public:
		// log message
		virtual void logMessage(Level level, const std::string &msg)override ;
	signals:
		void postMessage(int level, QString);
	private slots:
		void onLogMessage(int level, QString);
		void onClearMessage();

		// node tree widget show menu
		void showMenu(const QPoint& point);

		// copy log content
		void copyLogContent();

	private:
		uint32_t			m_sameMessageNum;
		std::string 		m_lastMessage;
		int					m_lastLevel;
		QMenu*				m_menu;
		QListWidgetItem*	m_currentSelectItem;
	};
}