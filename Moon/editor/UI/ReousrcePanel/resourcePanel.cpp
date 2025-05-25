#include "resourcePanel.h"
#include "QDirectoryModel.h"
#include "QPreviewHelper.h"
#include "QResListView.h"
#include <QFileSystemWatcher>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <filesystem>



QT_BEGIN_NAMESPACE


class Ui_ResPanel
{
public:
	QAction* m_actionNewFolder;
	QAction* m_actionShowInExplorer;
	QAction* m_actionDeleteRes;
	QAction* m_actionRenameRes;
	QAction* m_actionDuplicateRes;
	QAction* m_actionCopyPath;
	QWidget* dockWidgetContents;
	QVBoxLayout* verticalLayout_2;
	QSplitter* splitter;
	QTreeView* m_resDirView;
	QWidget* widget;
	QVBoxLayout* verticalLayout;
	QHBoxLayout* horizontalLayout;
	QLineEdit* m_searchLineEdit;
	QToolButton* m_viewTypeButton;
	MOON::QResListView* m_listView;

	void setupUi(QDockWidget* ResPanel)
	{
		if (ResPanel->objectName().isEmpty())
			ResPanel->setObjectName(QStringLiteral("ResPanel"));
		ResPanel->resize(241, 456);
		m_actionNewFolder = new QAction(ResPanel);
		m_actionNewFolder->setObjectName(QStringLiteral("m_actionNewFolder"));
		QIcon icon;
		icon.addFile(QStringLiteral(":/widgets/icons/folder.png"), QSize(), QIcon::Normal, QIcon::Off);
		m_actionNewFolder->setIcon(icon);
		m_actionShowInExplorer = new QAction(ResPanel);
		m_actionShowInExplorer->setObjectName(QStringLiteral("m_actionShowInExplorer"));
		m_actionShowInExplorer->setIcon(icon);
		m_actionDeleteRes = new QAction(ResPanel);
		m_actionDeleteRes->setObjectName(QStringLiteral("m_actionDeleteRes"));
		QIcon icon1;
		icon1.addFile(QStringLiteral(":/widgets/icons/delete.png"), QSize(), QIcon::Normal, QIcon::Off);
		m_actionDeleteRes->setIcon(icon1);
		m_actionRenameRes = new QAction(ResPanel);
		m_actionRenameRes->setObjectName(QStringLiteral("m_actionRenameRes"));
		m_actionDuplicateRes = new QAction(ResPanel);
		m_actionDuplicateRes->setObjectName(QStringLiteral("m_actionDuplicateRes"));
		m_actionCopyPath = new QAction(ResPanel);
		m_actionCopyPath->setObjectName(QStringLiteral("m_actionCopyPath"));
		dockWidgetContents = new QWidget();
		dockWidgetContents->setObjectName(QStringLiteral("dockWidgetContents"));
		verticalLayout_2 = new QVBoxLayout(dockWidgetContents);
		verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
		verticalLayout_2->setContentsMargins(0, 0, 0, 0);
		splitter = new QSplitter(dockWidgetContents);
		splitter->setObjectName(QStringLiteral("splitter"));
		splitter->setOrientation(Qt::Vertical);
		m_resDirView = new QTreeView(splitter);
		m_resDirView->setObjectName(QStringLiteral("m_resDirView"));
		m_resDirView->setHeaderHidden(true);
		splitter->addWidget(m_resDirView);
		widget = new QWidget(splitter);
		widget->setObjectName(QStringLiteral("widget"));
		verticalLayout = new QVBoxLayout(widget);
		verticalLayout->setSpacing(1);
		verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
		verticalLayout->setContentsMargins(0, 0, 0, 0);
		horizontalLayout = new QHBoxLayout();
		horizontalLayout->setSpacing(1);
		horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
		horizontalLayout->setContentsMargins(4, -1, 4, -1);
		m_searchLineEdit = new QLineEdit(widget);
		m_searchLineEdit->setObjectName(QStringLiteral("m_searchLineEdit"));

		horizontalLayout->addWidget(m_searchLineEdit);

		m_viewTypeButton = new QToolButton(widget);
		m_viewTypeButton->setObjectName(QStringLiteral("m_viewTypeButton"));
		QIcon icon2;
		icon2.addFile(QStringLiteral(":/widgets/icons/view_type_list.png"), QSize(), QIcon::Normal, QIcon::Off);
		m_viewTypeButton->setIcon(icon2);

		horizontalLayout->addWidget(m_viewTypeButton);


		verticalLayout->addLayout(horizontalLayout);

		m_listView = new MOON::QResListView(widget);
		m_listView->setObjectName(QStringLiteral("m_listView"));
		m_listView->setContextMenuPolicy(Qt::CustomContextMenu);
		m_listView->setAcceptDrops(true);
		m_listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		m_listView->setEditTriggers(QAbstractItemView::EditKeyPressed);
		m_listView->setDragEnabled(true);
		m_listView->setDragDropMode(QAbstractItemView::DragDrop);
		m_listView->setDefaultDropAction(Qt::MoveAction);

		verticalLayout->addWidget(m_listView);

		splitter->addWidget(widget);

		verticalLayout_2->addWidget(splitter);

		ResPanel->setWidget(dockWidgetContents);

		retranslateUi(ResPanel);

		QMetaObject::connectSlotsByName(ResPanel);
	} // setupUi

	void retranslateUi(QDockWidget* ResPanel)
	{
		ResPanel->setWindowTitle(QApplication::translate("ResPanel", "Resource", nullptr));
		m_actionNewFolder->setText(QApplication::translate("ResPanel", "Folder", nullptr));
		m_actionShowInExplorer->setText(QApplication::translate("ResPanel", "Show In Explorer", nullptr));
		m_actionDeleteRes->setText(QApplication::translate("ResPanel", "Delete", nullptr));
		m_actionRenameRes->setText(QApplication::translate("ResPanel", "Rename", nullptr));
		m_actionDuplicateRes->setText(QApplication::translate("ResPanel", "Duplicate", nullptr));
		m_actionCopyPath->setText(QApplication::translate("ResPanel", "Copy Path", nullptr));
#ifndef QT_NO_TOOLTIP
		m_actionCopyPath->setToolTip(QApplication::translate("ResPanel", "Copy Resource Path", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
		m_viewTypeButton->setToolTip(QApplication::translate("ResPanel", "List", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
		m_viewTypeButton->setStatusTip(QString());
#endif // QT_NO_STATUSTIP
		m_viewTypeButton->setText(QApplication::translate("ResPanel", "...", nullptr));
	} // retranslateUi

};

namespace Ui {
	class ResPanel : public Ui_ResPanel {};
} // namespace Ui

QT_END_NAMESPACE

namespace MOON {
	class ResPanel::ResPanelInternal {
	public:
		ResPanel::ResPanelInternal() {
			ui = new Ui_ResPanel();
		}
		~ResPanelInternal() {
			delete ui;
		}
		Ui_ResPanel* ui = nullptr;
		std::string m_currentDir;
		QDirectoryModel* m_dirModel;
		QPreviewHelper* m_previewHelper;
		QMenu* m_resMenu = nullptr;			// Mouse right button click
		QStandardItem* m_menuEditItem = nullptr;
		bool						m_viewTypeGrid = true;
		QFileSystemWatcher* m_filesystemWatcher = nullptr;
	private:


	};
	ResPanel::ResPanel(QWidget* parent) : QDockWidget(parent)
	{
		internal = new ResPanel::ResPanelInternal();
		internal->ui->setupUi(this);
		internal->m_dirModel = new QDirectoryModel();
		internal->m_dirModel->SetIcon("root", QIcon(":/widgets/icons/root.png"));
		internal->m_dirModel->SetIcon("filter", QIcon(":/widgets/icons/folder_close.png"));
		internal->m_dirModel->SetIcon("filterexpend", QIcon(":/widgets/icons/folder_open.png"));
		internal->ui->m_resDirView->setModel(internal->m_dirModel);
		internal->ui->m_resDirView->setAttribute(Qt::WA_MacShowFocusRect, 0);
		internal->m_dirModel->Clean();

		QObject::connect(internal->m_dirModel, SIGNAL(FileSelected(const char*)), this, SLOT(onSelectDir(const char*)));

		internal->m_previewHelper = new QPreviewHelper(internal->ui->m_listView);

		QObject::connect(internal->m_previewHelper, SIGNAL(clickedRes(const char*)), this, SLOT(onClickedPreviewRes(const char*)));
		QObject::connect(internal->m_previewHelper, SIGNAL(doubleClickedRes(const char*)), this, SLOT(onDoubleClickedPreviewRes(const char*)));
		QObject::connect(internal->m_previewHelper, SIGNAL(renamedRes(const QString, const QString)), this, SLOT(onRenamedRes(const QString, const QString)));
		QObject::connect(internal->ui->m_listView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showMenu(const QPoint&)));
		QObject::connect(internal->ui->m_actionShowInExplorer, SIGNAL(triggered()), this, SLOT(showInExporer()));
		QObject::connect(internal->ui->m_actionNewFolder, SIGNAL(triggered()), this, SLOT(newFolder()));
		QObject::connect(internal->ui->m_actionRenameRes, SIGNAL(triggered()), this, SLOT(onRenameRes()));
		QObject::connect(internal->ui->m_actionDeleteRes, SIGNAL(triggered()), this, SLOT(onDeleteRes()));
		QObject::connect(internal->ui->m_actionDuplicateRes, SIGNAL(triggered()), this, SLOT(onDuplicateRes()));
		QObject::connect(internal->ui->m_actionCopyPath, SIGNAL(triggered()), this, SLOT(onCopyResPath()));
		QObject::connect(internal->ui->m_viewTypeButton, SIGNAL(clicked()), this, SLOT(onSwitchResVeiwType()));
		QObject::connect(internal->ui->m_searchLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onSearchTextChanged()));
		onOpenProject();
	}
	ResPanel::~ResPanel()
	{
		delete internal;
	}
	ResPanel* ResPanel::instance()
	{
		return nullptr;
	}
	void ResPanel::onOpenProject()
	{
		internal->m_dirModel->clear();

		QDirectoryModel::RootPathArray rootPathes =
		{
			{"Res://", PROJECT_EDITOR_PATH, true},
			{"User://", PATH_TRACE_SCENE_PATH, true},
			{"Engine://", PROJECT_ENGINE_PATH, false},
		};

		internal->m_dirModel->setRootPath(rootPathes, "none", internal->ui->m_resDirView, NULL);
		internal->m_dirModel->Refresh();

		onSelectDir(PROJECT_EDITOR_PATH);

		resizeEvent(nullptr);
	}
	void ResPanel::recoverEditSettings()
	{
	}
	void ResPanel::onSelectDir(const char* dir) {
		internal->m_currentDir = dir;

		internal->m_previewHelper->clear();

		bool isIncludePreDir = dir == PROJECT_EDITOR_PATH ? false : true;
		internal->m_previewHelper->setPath(dir, nullptr, isIncludePreDir);
	}
	void ResPanel::onSelectFile(const char* pathName)
	{
	}
	void ResPanel::reslectCurrentDir()
	{
	}
	void ResPanel::onClickedPreviewRes(const char* res)
	{
	}
	void ResPanel::onDoubleClickedPreviewRes(const char* res)
	{
		std::filesystem::directory_entry p_directory(res);
		if (p_directory.is_directory()) {

			internal->m_dirModel->setCurrentSelect(res);
		}
	}
	void ResPanel::showMenu(const QPoint& point)
	{
	}
	void ResPanel::showInExporer()
	{
	}
	void ResPanel::newFolder()
	{
	}
	void ResPanel::onCreateRes()
	{
	}
	void ResPanel::onImportRes()
	{
	}
	void ResPanel::onRenameRes()
	{
	}
	void ResPanel::onDeleteRes()
	{
	}
	void ResPanel::onDuplicateRes()
	{
	}
	void ResPanel::onCopyResPath()
	{
	}
	void ResPanel::onRenamedRes(const QString src, const QString dest)
	{
	}
	void ResPanel::onSwitchResVeiwType()
	{
	}
	void ResPanel::onSearchTextChanged()
	{
	}
	void ResPanel::onWatchFileChanged(const QString& file)
	{
	}
	void ResPanel::onWatchFileDirChanged(const QString& dir)
	{
	}
	void ResPanel::resizeEvent(QResizeEvent* e)
	{
	}
}



