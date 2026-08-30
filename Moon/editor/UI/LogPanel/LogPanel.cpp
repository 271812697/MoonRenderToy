#include "LogPanel.h"
#include "core/log.h"
#include "editor/UI/DockWidgetTitleBar.h"

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QDateTime>
#include <QHBoxLayout>
#include <QMenu>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>

namespace MOON {

	namespace {
		QString levelColor(int level) {
			switch (level) {
				case LogOutput::LL_WARNING:
					return "#e5c07b";
				case LogOutput::LL_ERROR:
				case LogOutput::LL_FATAL:
					return "#e06c75";
				case LogOutput::LL_DEBUG:
					return "#7f8c98";
				default:
					return "#c8ccd0";
			}
		}
		QString levelName(int level) {
			switch (level) {
				case LogOutput::LL_DEBUG:
					return "debug";
				case LogOutput::LL_WARNING:
					return "warning";
				case LogOutput::LL_ERROR:
					return "error";
				case LogOutput::LL_FATAL:
					return "fatal";
				default:
					return "info";
			}
		}
	}

	LogPanel::LogPanel(QWidget* parent)
		: QDockWidget(parent), LogOutput("LogPanel") {
		setWindowTitle(tr("Log"));
		setTitleBarWidget(new DockWidgetTitleBar(this));

		auto* content = new QWidget(this);
		auto* mainLayout = new QHBoxLayout(content);
		mainLayout->setContentsMargins(3, 3, 3, 3);
		mainLayout->setSpacing(3);

		// Single rich-text view instead of one widget per log line: keeps the
		// panel fast even with a high message rate. The block count is capped
		// so memory stays bounded.
		m_logText = new QPlainTextEdit(content);
		m_logText->setReadOnly(true);
		m_logText->setMaximumBlockCount(10000);
		m_logText->setContextMenuPolicy(Qt::CustomContextMenu);
		mainLayout->addWidget(m_logText, 1);

		auto* sideLayout = new QVBoxLayout();
		sideLayout->setSpacing(3);
		m_debugCheck = new QCheckBox(tr("Debug"), content);
		m_debugCheck->setChecked(true);
		m_infoCheck = new QCheckBox(tr("Info"), content);
		m_infoCheck->setChecked(true);
		m_errorCheck = new QCheckBox(tr("Error"), content);
		m_errorCheck->setChecked(true);
		m_warnCheck = new QCheckBox(tr("Warning"), content);
		m_warnCheck->setChecked(true);
		m_clear = new QPushButton(tr("Clean"), content);
		sideLayout->addWidget(m_debugCheck);
		sideLayout->addWidget(m_infoCheck);
		sideLayout->addWidget(m_errorCheck);
		sideLayout->addWidget(m_warnCheck);
		sideLayout->addWidget(m_clear);
		sideLayout->addStretch();
		mainLayout->addLayout(sideLayout);

		setWidget(content);

		connect(this, &LogPanel::postMessage, this, &LogPanel::onLogMessage, Qt::QueuedConnection);
		connect(m_clear, &QPushButton::clicked, this, &LogPanel::onClearMessage);
		connect(m_debugCheck, &QCheckBox::toggled, this, &LogPanel::rebuildView);
		connect(m_infoCheck, &QCheckBox::toggled, this, &LogPanel::rebuildView);
		connect(m_errorCheck, &QCheckBox::toggled, this, &LogPanel::rebuildView);
		connect(m_warnCheck, &QCheckBox::toggled, this, &LogPanel::rebuildView);
		connect(m_logText, &QPlainTextEdit::customContextMenuRequested,
		        this, &LogPanel::showMenu);

		Log::intance().addOutput(this);
	}

	LogPanel::~LogPanel() {
		Log::intance().removeOutput(this);
	}

	void LogPanel::logMessage(Level level, const std::string& msg) {
		emit postMessage(level, QString::fromStdString(msg));
	}

	void LogPanel::onLogMessage(int level, QString msg) {
		m_messages.push_back({ level, msg });
		if (static_cast<int>(m_messages.size()) > 10000) {
			m_messages.pop_front();
		}
		if (levelVisible(level)) {
			appendMessage(level, msg);
		}
	}

	void LogPanel::appendMessage(int level, const QString& msg) {
		const QString time = QDateTime::currentDateTime().toString("yyyy/M/d H:mm:ss");
		// Trim the message: trailing/leading whitespace would otherwise be shown
		// as an HTML entity or collapse when the rich text is imported.
		const QString text = msg.trimmed();
		const QString html = QString("<span style='color:%1;'>[%2] > [%3]: %4</span>")
		                         .arg(levelColor(level))
		                         .arg(levelName(level))
		                         .arg(time)
		                         .arg(text.toHtmlEscaped());
		m_logText->appendHtml(html);
		m_logText->verticalScrollBar()->setValue(m_logText->verticalScrollBar()->maximum());
	}

	bool LogPanel::levelVisible(int level) const {
		switch (level) {
			case LogOutput::LL_DEBUG:
				return m_debugCheck->isChecked();
			case LogOutput::LL_INFO:
				return m_infoCheck->isChecked();
			case LogOutput::LL_WARNING:
				return m_warnCheck->isChecked();
			case LogOutput::LL_ERROR:
			case LogOutput::LL_FATAL:
				return m_errorCheck->isChecked();
			default:
				return true;
		}
	}

	void LogPanel::rebuildView() {
		m_logText->setUpdatesEnabled(false);
		m_logText->clear();
		for (const auto& [level, msg] : m_messages) {
			if (levelVisible(level)) {
				appendMessage(level, msg);
			}
		}
		m_logText->setUpdatesEnabled(true);
	}

	void LogPanel::onClearMessage() {
		m_messages.clear();
		m_logText->clear();
	}

	void LogPanel::showMenu(const QPoint&) {
		QMenu menu(this);
		QAction* copy = menu.addAction(tr("Copy"));
		connect(copy, &QAction::triggered, this, &LogPanel::copyLogContent);
		menu.exec(QCursor::pos());
	}

	void LogPanel::copyLogContent() {
		QApplication::clipboard()->setText(m_logText->textCursor().selectedText());
	}

}
