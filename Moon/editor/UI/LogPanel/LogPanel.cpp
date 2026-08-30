#include "LogPanel.h"
#include "core/log.h"
#include "editor/UI/DockWidgetTitleBar.h"

#include <QAbstractItemModel>
#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QColor>
#include <QDateTime>
#include <QFontMetrics>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QMenu>
#include <QPushButton>
#include <QScrollBar>
#include <QTableView>
#include <QVBoxLayout>

#include <array>
#include <deque>
#include <vector>

namespace MOON {

	namespace {
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
		QColor levelColor(int level) {
			switch (level) {
				case LogOutput::LL_WARNING:
					return QColor("#e5c07b");
				case LogOutput::LL_ERROR:
				case LogOutput::LL_FATAL:
					return QColor("#e06c75");
				case LogOutput::LL_DEBUG:
					return QColor("#7f8c98");
				default:
					return QColor("#98c379");
			}
		}
	}

	// ---------------------------------------------------------------------------
	// LogTableModel: ring buffer of log entries exposed as a three-column table.
	// ---------------------------------------------------------------------------
	class LogTableModel : public QAbstractTableModel {
	public:
		enum Column { Level = 0, Time = 1, Message = 2 };
		static constexpr int MaxEntries = 10000;

		explicit LogTableModel(QObject* parent = nullptr)
			: QAbstractTableModel(parent) {
			m_visibleByLevel.fill(true);
		}

		int rowCount(const QModelIndex& parent = QModelIndex()) const override {
			return parent.isValid() ? 0 : static_cast<int>(m_visible.size());
		}
		int columnCount(const QModelIndex& parent = QModelIndex()) const override {
			return parent.isValid() ? 0 : 3;
		}

		QVariant data(const QModelIndex& index, int role) const override {
			if (!index.isValid() || index.row() < 0 ||
			    index.row() >= static_cast<int>(m_visible.size())) {
				return {};
			}
			const auto& entry = m_entries[m_visible[index.row()]];
			switch (role) {
				case Qt::DisplayRole:
					switch (index.column()) {
						case Level:
							return levelName(entry.level);
						case Time:
							return entry.time;
						case Message:
							return entry.message;
					}
					break;
				case Qt::ForegroundRole:
					return QBrush(levelColor(entry.level));
				default:
					break;
			}
			return {};
		}

		QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
			if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
				switch (section) {
					case Level:
						return "Level";
					case Time:
						return "Time";
					case Message:
						return "Message";
				}
			}
			return {};
		}

		void append(int level, const QString& time, const QString& message) {
			m_entries.push_back({ level, time, message });
			if (static_cast<int>(m_entries.size()) > MaxEntries) {
				m_entries.pop_front();
				rebuildVisible();
				return;
			}
			if (levelVisible(level)) {
				const int row = static_cast<int>(m_visible.size());
				beginInsertRows(QModelIndex(), row, row);
				m_visible.push_back(static_cast<int>(m_entries.size()) - 1);
				endInsertRows();
			}
		}

		void setLevelVisible(int level, bool visible) {
			if (level >= 0 && level < static_cast<int>(m_visibleByLevel.size()) &&
			    m_visibleByLevel[level] != visible) {
				m_visibleByLevel[level] = visible;
				rebuildVisible();
			}
		}

		void clear() {
			beginResetModel();
			m_entries.clear();
			m_visible.clear();
			endResetModel();
		}

	private:
		struct Entry {
			int level = LogOutput::LL_INFO;
			QString time;
			QString message;
		};

		bool levelVisible(int level) const {
			if (level < 0 || level >= static_cast<int>(m_visibleByLevel.size())) {
				return true;
			}
			return m_visibleByLevel[level];
		}

		void rebuildVisible() {
			beginResetModel();
			m_visible.clear();
			for (int i = 0; i < static_cast<int>(m_entries.size()); ++i) {
				if (levelVisible(m_entries[i].level)) {
					m_visible.push_back(i);
				}
			}
			endResetModel();
		}

		std::deque<Entry> m_entries;
		std::vector<int> m_visible;  // deque indices currently shown
		std::array<bool, 5> m_visibleByLevel;  // LL_DEBUG..LL_FATAL
	};

	// ---------------------------------------------------------------------------
	// LogPanel
	// ---------------------------------------------------------------------------
	LogPanel::LogPanel(QWidget* parent)
		: QDockWidget(parent), LogOutput("LogPanel") {
		setWindowTitle(tr("Log"));
		setTitleBarWidget(new DockWidgetTitleBar(this));

		m_model = new LogTableModel(this);

		auto* content = new QWidget(this);
		auto* mainLayout = new QHBoxLayout(content);
		mainLayout->setContentsMargins(3, 3, 3, 3);
		mainLayout->setSpacing(3);

		m_logView = new QTableView(content);
		m_logView->setModel(m_model);
		m_logView->setSelectionBehavior(QAbstractItemView::SelectRows);
		m_logView->setSelectionMode(QAbstractItemView::ExtendedSelection);
		m_logView->setEditTriggers(QAbstractItemView::NoEditTriggers);
		m_logView->setShowGrid(false);
		m_logView->setWordWrap(false);
		m_logView->setContextMenuPolicy(Qt::CustomContextMenu);
		m_logView->verticalHeader()->hide();
		auto* header = m_logView->horizontalHeader();
		header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		header->setSectionResizeMode(LogTableModel::Level, QHeaderView::ResizeToContents);
		header->setSectionResizeMode(LogTableModel::Time, QHeaderView::Fixed);
		header->setSectionResizeMode(LogTableModel::Message, QHeaderView::Stretch);
		// Wide enough for the full timestamp, plus a visible gap before the
		// message column.
		const QFontMetrics fm(m_logView->font());
		header->resizeSection(LogTableModel::Time,
		                      fm.horizontalAdvance(QStringLiteral("2026/8/31 0:53:46")) + 40);
		mainLayout->addWidget(m_logView, 1);

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
		connect(m_debugCheck, &QCheckBox::toggled, this, [this](bool on) {
			m_model->setLevelVisible(LogOutput::LL_DEBUG, on);
			m_logView->scrollToBottom();
		});
		connect(m_infoCheck, &QCheckBox::toggled, this, [this](bool on) {
			m_model->setLevelVisible(LogOutput::LL_INFO, on);
			m_logView->scrollToBottom();
		});
		connect(m_warnCheck, &QCheckBox::toggled, this, [this](bool on) {
			m_model->setLevelVisible(LogOutput::LL_WARNING, on);
			m_logView->scrollToBottom();
		});
		connect(m_errorCheck, &QCheckBox::toggled, this, [this](bool on) {
			m_model->setLevelVisible(LogOutput::LL_ERROR, on);
			m_model->setLevelVisible(LogOutput::LL_FATAL, on);
			m_logView->scrollToBottom();
		});
		connect(m_logView, &QTableView::customContextMenuRequested,
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
		// Stick to the bottom if the user is already there.
		auto* scrollBar = m_logView->verticalScrollBar();
		const bool stickToBottom =
			scrollBar->value() >= scrollBar->maximum() - 2 || scrollBar->maximum() == 0;
		m_model->append(level, QDateTime::currentDateTime().toString("yyyy/M/d H:mm:ss"),
		                msg.trimmed());
		if (stickToBottom) {
			m_logView->scrollToBottom();
		}
	}

	void LogPanel::onClearMessage() {
		m_model->clear();
	}

	void LogPanel::showMenu(const QPoint&) {
		QMenu menu(this);
		QAction* copy = menu.addAction(tr("Copy"));
		connect(copy, &QAction::triggered, this, &LogPanel::copyLogContent);
		menu.exec(QCursor::pos());
	}

	void LogPanel::copyLogContent() {
		QString text;
		const auto rows = m_logView->selectionModel()->selectedRows();
		for (const QModelIndex& row : rows) {
			const QString level =
				m_model->data(m_model->index(row.row(), LogTableModel::Level), Qt::DisplayRole).toString();
			const QString time =
				m_model->data(m_model->index(row.row(), LogTableModel::Time), Qt::DisplayRole).toString();
			const QString message =
				m_model->data(m_model->index(row.row(), LogTableModel::Message), Qt::DisplayRole).toString();
			text += level + "\t" + time + "\t" + message + "\n";
		}
		QApplication::clipboard()->setText(text);
	}

}
