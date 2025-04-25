#include "pqFileDialog.h"
#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAction>
#include <QComboBox>
#include <QCompleter>
#include <QDesktopServices>
#include <QDir>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPoint>
#include <QRegularExpression>
#include <QScopedValueRollback>
#include <QShortcut>
#include <QShowEvent>
#include <QTabWidget>
#include <QtDebug>
#include <string>

#include "uipqFileDialog.h"
#include <QtGlobal>

namespace
{

	QStringList MakeFilterList(const QString& filter)
	{
		if (filter.contains(";;"))
		{
			return filter.split(";;", Qt::SkipEmptyParts);
		}

		return filter.split('\n', Qt::SkipEmptyParts);
	}

	QStringList GetWildCardsFromFilter(const QString& filter)
	{
		QString f = filter;
		// if we have (...) in our filter, strip everything out but the contents of ()
		int start, end;
		start = filter.indexOf('(');
		end = filter.lastIndexOf(')');
		if (start != -1 && end != -1)
		{
			f = f.mid(start + 1, end - start - 1);
		}
		else if (start != -1 || end != -1)
		{
			f = QString(); // hmm...  I'm confused
		}

		// separated by spaces or semi-colons
		QStringList fs = f.split(QRegularExpression("[\\s+;]"), Qt::SkipEmptyParts);

		// add a *.ext.* for every *.ext we get to support file groups
		QStringList ret = fs;
		Q_FOREACH(QString ext, fs)
		{
			ret.append(ext + ".*");
		}
		return ret;
	}
}

/////////////////////////////////////////////////////////////////////////////
// pqFileDialog::pqImplementation

class pqFileDialog::pqImplementation : public QWidget
{
public:
	//pqFileDialogModel* const Model;
	//pqFileDialogFavoriteModel* const FavoriteModel;
	//pqFileDialogLocationModel* const LocationModel;
	//pqFileDialogRecentDirsModel* const RecentModel;
	//QSortFilterProxyModel* proxyFavoriteModel;
	//pqFileDialogFilter FileFilter;
	QStringList FileNames; // list of file names in the FileName ui text edit
	QCompleter* Completer;
	pqFileDialog::FileMode Mode;
	Ui::pqFileDialog Ui;
	QList<QStringList> SelectedFiles;
	int SelectedFilterIndex;
	QStringList Filters;
	bool SuppressOverwriteWarning;
	bool ShowMultipleFileHelp;
	bool SupportsGroupFiles = true;
	QString FileNamesSeperator;
	bool InDoubleClickHandler; //< used to determine if we're "accept"ing as a result of
	//  double-clicking as that elicits a different
	//  response.

// remember the last locations we browsed
	//static QMap<QPointer<pqServer>, QString> FilePaths;

	pqImplementation(pqFileDialog* p)
		: QWidget(p)
		//, Model(new pqFileDialogModel(server, nullptr))
		//, FavoriteModel(new pqFileDialogFavoriteModel(Model, server, nullptr))
		//, LocationModel(new pqFileDialogLocationModel(Model, server, nullptr))
		//, RecentModel(new pqFileDialogRecentDirsModel(Model, server, nullptr))
		//, FileFilter(this->Model)
		//, Completer(new QCompleter(&this->FileFilter, nullptr))
		, Mode(ExistingFile)
		, SuppressOverwriteWarning(false)
		, ShowMultipleFileHelp(false)
		, FileNamesSeperator(";")
		, InDoubleClickHandler(false)
	{
		//QObject::connect(p, SIGNAL(filesSelected(const QList<QStringList>&)), this->RecentModel,
			//SLOT(setChosenFiles(const QList<QStringList>&)));
	}

	~pqImplementation() override
	{
		//delete this->RecentModel;
		//delete this->LocationModel;
		//delete this->FavoriteModel;
		//delete this->Model;
		//delete this->Completer;
	}

	bool eventFilter(QObject* obj, QEvent* anEvent) override
	{
		if (obj == this->Ui.Files)
		{
			if (anEvent->type() == QEvent::KeyPress)
			{
				QKeyEvent* keyEvent = static_cast<QKeyEvent*>(anEvent);
				if (keyEvent->key() == Qt::Key_Backspace || keyEvent->key() == Qt::Key_Delete)
				{
					this->Ui.EntityName->setFocus(Qt::OtherFocusReason);
					// send out a backspace event to the file name now
					QKeyEvent replicateDelete(keyEvent->type(), keyEvent->key(), keyEvent->modifiers());
					QApplication::sendEvent(this->Ui.EntityName, &replicateDelete);
					return true;
				}
			}
			return false;
		}
		return QObject::eventFilter(obj, anEvent);
	}

	QString getStartPath()
	{
		return "";// this->Model->getCurrentPath();
	}

	void setCurrentPath(const QString& p)
	{
		//this->Model->setCurrentPath(p);
		//pqServer* s = this->Model->server();
		//this->FilePaths[s] = p;
		this->Ui.Favorites->clearSelection();
		this->Ui.Locations->clearSelection();
		this->Ui.Recent->clearSelection();
		this->Ui.Files->setFocus(Qt::OtherFocusReason);
	}

	void addHistory(const QString& p)
	{
		this->BackHistory.append(p);
		this->ForwardHistory.clear();
		this->Ui.NavigateBack->setEnabled(this->BackHistory.size() > 1);
		this->Ui.NavigateForward->setEnabled(false);
	}
	QString backHistory()
	{
		QString path = this->BackHistory.takeLast();
		//this->ForwardHistory.append(this->Model->getCurrentPath());
		this->Ui.NavigateForward->setEnabled(true);
		if (this->BackHistory.size() == 1)
		{
			this->Ui.NavigateBack->setEnabled(false);
		}
		return path;
	}
	QString forwardHistory()
	{
		QString path = this->ForwardHistory.takeLast();
		//this->BackHistory.append(this->Model->getCurrentPath());
		this->Ui.NavigateBack->setEnabled(true);
		if (this->ForwardHistory.empty())
		{
			this->Ui.NavigateForward->setEnabled(false);
		}
		return path;
	}

protected:
	QStringList BackHistory;
	QStringList ForwardHistory;
};



/////////////////////////////////////////////////////////////////////////////
void pqFileDialog::addImplementation(unsigned int location)
{

	this->Implementations[location] = new pqImplementation(this);
	this->Implementations[location]->setObjectName(
		QString("Filesystem_%1").arg(this->Implementations.size() - 1));
	// the selected location is temporarily set here,
	// so that some slots called by signals can be executed properly.
	this->SelectedLocation = location;
	auto& impl = *this->Implementations[location];

	// set up ui for the file system
	this->Implementations[location]->Ui.setupUi(this->Implementations[location]);

	// set up ok and cancel signals/slots
	QObject::connect(impl.Ui.OK, &QPushButton::clicked, this, &pqFileDialog::accept);
	QObject::connect(impl.Ui.Cancel, &QPushButton::clicked, this, &pqFileDialog::reject);

	// ensures that the favorites and the browser component are sized proportionately.
	impl.Ui.mainSplitter->setStretchFactor(0, 1);
	impl.Ui.mainSplitter->setStretchFactor(1, 4);

	impl.Ui.Files->setEditTriggers(QAbstractItemView::EditKeyPressed);

	// install the event filter
	impl.Ui.Files->installEventFilter(this->Implementations[location]);

	// install the autocompleter
	//impl.Ui.EntityName->setCompleter(impl.Completer);

	// this is the Navigate button, which is only shown when needed
	impl.Ui.Navigate->hide();

	QPixmap back = style()->standardPixmap(QStyle::SP_FileDialogBack);
	impl.Ui.NavigateBack->setIcon(back);
	impl.Ui.NavigateBack->setEnabled(false);
	impl.Ui.NavigateBack->setShortcut(QKeySequence::Back);
	impl.Ui.NavigateBack->setToolTip(
		tr("Navigate Back (%1)").arg(impl.Ui.NavigateBack->shortcut().toString()));

	QObject::connect(impl.Ui.NavigateBack, SIGNAL(clicked(bool)), this, SLOT(onNavigateBack()));
	// just flip the back image to make a forward image
	QPixmap forward = QPixmap::fromImage(back.toImage().mirrored(true, false));
	impl.Ui.NavigateForward->setIcon(forward);
	impl.Ui.NavigateForward->setDisabled(true);
	impl.Ui.NavigateForward->setShortcut(QKeySequence::Forward);
	impl.Ui.NavigateForward->setToolTip(
		tr("Navigate Forward (%1)").arg(impl.Ui.NavigateForward->shortcut().toString()));
	QObject::connect(impl.Ui.NavigateForward, SIGNAL(clicked(bool)), this, SLOT(onNavigateForward()));
	impl.Ui.NavigateUp->setIcon(style()->standardPixmap(QStyle::SP_FileDialogToParent));
	impl.Ui.NavigateUp->setShortcut(Qt::ALT | Qt::Key_Up);
	impl.Ui.NavigateUp->setToolTip(
		tr("Navigate Up (%1)").arg(impl.Ui.NavigateUp->shortcut().toString()));
	impl.Ui.CreateFolder->setIcon(style()->standardPixmap(QStyle::SP_FileDialogNewFolder));
	impl.Ui.CreateFolder->setShortcut(QKeySequence::New);
	impl.Ui.CreateFolder->setToolTip(
		tr("Create New Folder (%1)").arg(impl.Ui.CreateFolder->shortcut().toString()));

	impl.Ui.ShowDetail->setIcon(QIcon(":/widgets/icons/pqAdvanced.svg"));

	//impl.Ui.Files->setModel(&impl.FileFilter);
	impl.Ui.Files->setSelectionBehavior(QAbstractItemView::SelectRows);

	impl.Ui.Files->setContextMenuPolicy(Qt::CustomContextMenu);
	QObject::connect(impl.Ui.Files, SIGNAL(customContextMenuRequested(const QPoint&)), this,
		SLOT(onContextMenuRequested(const QPoint&)));

	impl.Ui.Favorites->setContextMenuPolicy(Qt::CustomContextMenu);
	QObject::connect(impl.Ui.Favorites, SIGNAL(customContextMenuRequested(const QPoint&)), this,
		SLOT(onFavoritesContextMenuRequested(const QPoint&)));

	impl.Ui.Favorites->setEditTriggers(QAbstractItemView::EditTrigger::EditKeyPressed);

	auto shortcutDel = new QShortcut(QKeySequence::Delete, this);
	shortcutDel->setAutoRepeat(false);
	QObject::connect(shortcutDel, &QShortcut::activated, this,
		&pqFileDialog::onRemoveSelectedDirectoriesFromFavorites);

	impl.Ui.AddCurrentDirectoryToFavorites->setIcon(QIcon(":/widgets/icons/pqPlus.svg"));
	QObject::connect(impl.Ui.AddCurrentDirectoryToFavorites, SIGNAL(clicked()), this,
		SLOT(onAddCurrentDirectoryToFavorites()));
	impl.Ui.ResetFavortiesToSystemDefault->setIcon(QIcon(":/widgets/icons/pqReset.svg"));
	QObject::connect(impl.Ui.ResetFavortiesToSystemDefault, SIGNAL(clicked()), this,
		SLOT(onResetFavoritesToSystemDefault()));

	//impl.proxyFavoriteModel = new QSortFilterProxyModel(impl.FavoriteModel);
	//impl.proxyFavoriteModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
	///impl.proxyFavoriteModel->setSourceModel(impl.FavoriteModel);

	//impl.Ui.Favorites->setModel(impl.proxyFavoriteModel);
	//impl.Ui.Favorites->setSelectionMode(QAbstractItemView::ExtendedSelection);

	//impl.Ui.Locations->setModel(impl.LocationModel);
	//impl.Ui.Recent->setModel(impl.RecentModel);

	this->setFileMode(ExistingFile, location);

	//QObject::connect(impl.Model, SIGNAL(modelReset()), this, SLOT(onModelReset()));

	QObject::connect(impl.Ui.NavigateUp, SIGNAL(clicked()), this, SLOT(onNavigateUp()));

	QObject::connect(impl.Ui.CreateFolder, SIGNAL(clicked()), this, SLOT(onCreateNewFolder()));

	QObject::connect(
		impl.Ui.Parents, SIGNAL(activated(const QString&)), this, SLOT(onNavigate(const QString&)));

	QObject::connect(
		impl.Ui.EntityType, &QComboBox::currentTextChanged, this, &pqFileDialog::onFilterChange);

	QObject::connect(impl.Ui.Favorites, SIGNAL(clicked(const QModelIndex&)), this,
		SLOT(onClickedFavorite(const QModelIndex&)));

	QObject::connect(impl.Ui.favoritesSearchBar, &QLineEdit::textChanged, this,
		&pqFileDialog::FilterDirectoryFromFavorites);

	QObject::connect(impl.Ui.Recent, SIGNAL(clicked(const QModelIndex&)), this,
		SLOT(onClickedRecent(const QModelIndex&)));
	QObject::connect(impl.Ui.Locations, SIGNAL(clicked(const QModelIndex&)), this,
		SLOT(onClickedRecent(const QModelIndex&)));

	QObject::connect(impl.Ui.Files, SIGNAL(clicked(const QModelIndex&)), this,
		SLOT(onClickedFile(const QModelIndex&)));

	QObject::connect(impl.Ui.Files->selectionModel(),
		SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this,
		SLOT(fileSelectionChanged()));

	QObject::connect(impl.Ui.Favorites, SIGNAL(activated(const QModelIndex&)), this,
		SLOT(onActivateFavorite(const QModelIndex&)));

	QObject::connect(impl.Ui.Locations, SIGNAL(activated(const QModelIndex&)), this,
		SLOT(onActivateLocation(const QModelIndex&)));
	QObject::connect(impl.Ui.Recent, SIGNAL(activated(const QModelIndex&)), this,
		SLOT(onActivateRecent(const QModelIndex&)));

	QObject::connect(impl.Ui.Files, SIGNAL(doubleClicked(const QModelIndex&)), this,
		SLOT(onDoubleClickFile(const QModelIndex&)));

	QObject::connect(impl.Ui.EntityName, SIGNAL(textChanged(const QString&)), this,
		SLOT(onTextEdited(const QString&)));

	//impl.Completer->setCaseSensitivity(Qt::CaseInsensitive);

	QStringList filterList = MakeFilterList(this->NameFilter);
	if (filterList.empty())
	{
		impl.Ui.EntityType->addItem(tr("All Files") + " (*)");
		impl.Filters << tr("All Files") + " (*)";
	}
	else
	{
		impl.Ui.EntityType->addItems(filterList);
		impl.Filters = filterList;
	}
	this->onFilterChange(impl.Ui.EntityType->currentText());

	QString startPath = this->StartDirectory;
	if (startPath.isEmpty() || (!startPath.isEmpty() && location != this->DefaultLocation))
	{
		startPath = impl.getStartPath();
	}
	impl.addHistory(startPath);
	impl.setCurrentPath(startPath);

	impl.Ui.Files->resizeColumnToContents(0);
	impl.Ui.Files->setTextElideMode(Qt::ElideMiddle);
	QHeaderView* header = impl.Ui.Files->header();

	// This code is similar to QFileDialog code
	// It positions different columns and orders in a standard way
	QFontMetrics fm(this->font());
	header->resizeSection(0, fm.horizontalAdvance(QLatin1String("wwwwwwwwwwwwwwwwwwwwwwwwww")));
	header->resizeSection(1, fm.horizontalAdvance(QLatin1String("mp3Folder")));
	header->resizeSection(2, fm.horizontalAdvance(QLatin1String("128.88 GB")));
	header->resizeSection(3, fm.horizontalAdvance(QLatin1String("10/29/81 02:02PM")));
	impl.Ui.Files->setSortingEnabled(true);
	impl.Ui.Files->header()->setSortIndicator(0, Qt::AscendingOrder);

	//bool showDetail = impl.Model->isShowingDetailedInfo();
	//impl.Ui.ShowDetail->setChecked(showDetail);
	//impl.Ui.Files->setColumnHidden(2, !showDetail);
	//impl.Ui.Files->setColumnHidden(3, !showDetail);

	// Group files handling
	impl.SupportsGroupFiles = this->SupportsGroupFiles;
	impl.Ui.GroupFiles->setEnabled(impl.SupportsGroupFiles);
	impl.Ui.GroupFiles->setVisible(impl.SupportsGroupFiles);
	this->connect(impl.Ui.GroupFiles, SIGNAL(clicked(bool)), this, SLOT(onGroupFilesToggled(bool)));

	// let's manage the default button.
	impl.Ui.OK->setDefault(true);
	impl.Ui.Navigate->setDefault(false);

	this->connect(impl.Ui.Navigate, SIGNAL(clicked()), SLOT(onNavigate()));

	// Use saved state if any
	this->restoreState(location);
}

/////////////////////////////////////////////////////////////////////////////
// pqFileDialog
pqFileDialog::pqFileDialog(QWidget* p, const QString& title,
	const QString& startDirectory, const QString& nameFilter, bool supportsGroupFiles,
	bool onlyBrowseRemotely)
	: Superclass(p)
	//, Server(server)
	, StartDirectory(startDirectory)
	, NameFilter(nameFilter)
	, SupportsGroupFiles(supportsGroupFiles)
{
	// remove do-nothing "?" title bar button on Windows.
	this->setWindowFlags(this->windowFlags().setFlag(Qt::WindowContextHelpButtonHint, false));
	this->setWindowTitle(title);
	this->setObjectName("pqFileDialog");

	// create a tab widget for the vtkPVSession::CLIENT and vtkPVSession::DATA_SERVER locations
	QPointer<QTabWidget> tabWidget = new QTabWidget(this);
	tabWidget->setObjectName("tabWidget");

	// check if local file system can only be used
	const bool canOnlyUseLocalFileSystem = true;
	// check if local file system can be used
	const bool enableLocalImplementation = !onlyBrowseRemotely ? true : canOnlyUseLocalFileSystem;
	// check if remote file system can be used
	const bool enableRemoteImplementation = !canOnlyUseLocalFileSystem;
	// compute default location
	this->DefaultLocation = 16;// !onlyBrowseRemotely || canOnlyUseLocalFileSystem
	//? vtkPVSession::CLIENT
	//: vtkPVSession::DATA_SERVER;

	std::vector<std::pair<unsigned int, QString>> locationsNames;
	if (enableLocalImplementation)
	{
		locationsNames.emplace_back(16, tr("Local File System"));
	}
	if (enableRemoteImplementation)
	{
		//locationsNames.emplace_back(vtkPVSession::DATA_SERVER, tr("Remote File System"));
	}
	for (const auto& locationName : locationsNames)
	{
		const unsigned int& location = locationName.first;
		const QString& name = locationName.second;
		// only add the default location, and create a placeholder for the non-default one
		if (location == this->DefaultLocation)
		{
			this->addImplementation(location);
			tabWidget->addTab(this->Implementations[location], name);
		}
		else
		{
			QPointer<QWidget> tempWidget = new QWidget();
			tabWidget->addTab(tempWidget, name);
		}
	}

	// set the QTabWidget as the central widget of the dialog
	QPointer<QVBoxLayout> layout = new QVBoxLayout(this);
	layout->addWidget(tabWidget);
	this->setLayout(layout);

	// set default location
	this->SelectedLocation = this->DefaultLocation;

	QObject::connect(tabWidget, &QTabWidget::currentChanged, this, &pqFileDialog::onLocationChanged);
}

//-----------------------------------------------------------------------------
pqFileDialog::~pqFileDialog()
{
	this->saveState();
}

//-----------------------------------------------------------------------------
void pqFileDialog::onCreateNewFolder()
{

}

//-----------------------------------------------------------------------------
void pqFileDialog::onContextMenuRequested(const QPoint& menuPos)
{

}

//-----------------------------------------------------------------------------
void pqFileDialog::AddDirectoryToFavorites(QString const& directory)
{

}

//-----------------------------------------------------------------------------
void pqFileDialog::RemoveDirectoryFromFavorites(QString const& directory)
{

}

//-----------------------------------------------------------------------------
void pqFileDialog::FilterDirectoryFromFavorites(const QString& filter)
{

}

//-----------------------------------------------------------------------------
void pqFileDialog::onAddCurrentDirectoryToFavorites()
{

}

//-----------------------------------------------------------------------------
void pqFileDialog::onRemoveSelectedDirectoriesFromFavorites()
{

}

//-----------------------------------------------------------------------------
void pqFileDialog::onResetFavoritesToSystemDefault()
{

}

//-----------------------------------------------------------------------------
void pqFileDialog::onFavoritesContextMenuRequested(const QPoint& menuPos)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void pqFileDialog::setFileMode(FileMode mode, unsigned int location)
{

}

//-----------------------------------------------------------------------------
void pqFileDialog::setFileMode(FileMode mode)
{

}

//-----------------------------------------------------------------------------
void pqFileDialog::setRecentlyUsedExtension(const QString& fileExtension, unsigned int location)
{
	auto& impl = *this->Implementations[location];
	this->RecentlyUsedExtension = fileExtension;

	if (fileExtension == QString())
	{
		// upon the initial use of any kind (e.g., data or screenshot) of dialog
		// 'fileExtension' is equal /set to an empty string.
		// In this case, no any user preferences are considered
		impl.Ui.EntityType->setCurrentIndex(0);
	}
	else
	{
		int index = impl.Ui.EntityType->findText(this->RecentlyUsedExtension, Qt::MatchContains);
		// just in case the provided extension is not in the combobox list
		index = (index == -1) ? 0 : index;
		impl.Ui.EntityType->setCurrentIndex(index);
	}
}

//-----------------------------------------------------------------------------
void pqFileDialog::setRecentlyUsedExtension(const QString& fileExtension)
{

}

//-----------------------------------------------------------------------------
void pqFileDialog::addToFilesSelected(const QStringList& files)
{
	auto& impl = *this->Implementations[this->SelectedLocation];

	// Ensure that we are hidden before broadcasting the selection,
	// so we don't get caught by screen-captures
	this->setVisible(false);
	impl.SelectedFiles.append(files);
}

//-----------------------------------------------------------------------------
void pqFileDialog::emitFilesSelectionDone()
{
	auto& impl = *this->Implementations[this->SelectedLocation];
	Q_EMIT filesSelected(impl.SelectedFiles);
	if (impl.Mode != this->ExistingFiles && !impl.SelectedFiles.empty())
	{
		Q_EMIT filesSelected(impl.SelectedFiles[0]);
	}
	this->done(QDialog::Accepted);
}

//-----------------------------------------------------------------------------
QList<QStringList> pqFileDialog::getAllSelectedFiles()
{
	auto& impl = *this->Implementations[this->SelectedLocation];
	return impl.SelectedFiles;
}

//-----------------------------------------------------------------------------
QStringList pqFileDialog::getSelectedFiles(int index)
{
	auto& impl = *this->Implementations[this->SelectedLocation];
	if (index < 0 || index >= impl.SelectedFiles.size())
	{
		return QStringList();
	}
	return impl.SelectedFiles[index];
}

//-----------------------------------------------------------------------------
int pqFileDialog::getSelectedFilterIndex()
{
	return this->Implementations[this->SelectedLocation]->SelectedFilterIndex;
}

//-----------------------------------------------------------------------------
void pqFileDialog::accept()
{
	auto& impl = *this->Implementations[this->SelectedLocation];

	bool loadedFile = false;
	switch (impl.Mode)
	{
	case AnyFile:
	case Directory:
		loadedFile = this->acceptDefault(false);
		break;
	case ExistingFiles:
	case ExistingFile:
	case ExistingFilesAndDirectories:
		loadedFile = this->acceptExistingFiles();
		break;
	}
	if (loadedFile)
	{
		Q_EMIT this->emitFilesSelectionDone();
	}
}

//-----------------------------------------------------------------------------
bool pqFileDialog::acceptExistingFiles()
{
	return false;
}

//-----------------------------------------------------------------------------
bool pqFileDialog::acceptDefault(const bool& checkForGrouping)
{
	return false;
}

//-----------------------------------------------------------------------------
QStringList pqFileDialog::buildFileGroup(const QString& filename)
{
	auto& impl = *this->Implementations[this->SelectedLocation];

	QStringList files;



	return files;
}

//-----------------------------------------------------------------------------
void pqFileDialog::onLocationChanged(int index)
{

}

//-----------------------------------------------------------------------------
void pqFileDialog::onModelReset()
{

}

//-----------------------------------------------------------------------------
void pqFileDialog::onNavigate(const QString& newpath)
{

}

//-----------------------------------------------------------------------------
void pqFileDialog::onNavigateUp()
{

}

//-----------------------------------------------------------------------------
void pqFileDialog::onNavigateDown(const QModelIndex& idx)
{

}

//-----------------------------------------------------------------------------
void pqFileDialog::onNavigateBack()
{
	auto& impl = *this->Implementations[this->SelectedLocation];
	QString path = impl.backHistory();
	impl.setCurrentPath(path);
}

//-----------------------------------------------------------------------------
void pqFileDialog::onNavigateForward()
{
	auto& impl = *this->Implementations[this->SelectedLocation];
	QString path = impl.forwardHistory();
	impl.setCurrentPath(path);
}

//-----------------------------------------------------------------------------
void pqFileDialog::onFilterChange(const QString& filter)
{

}

//-----------------------------------------------------------------------------
void pqFileDialog::onClickedFavorite(const QModelIndex&)
{
	auto& impl = *this->Implementations[this->SelectedLocation];
	impl.Ui.Files->clearSelection();
}

//-----------------------------------------------------------------------------
void pqFileDialog::onClickedRecent(const QModelIndex&)
{
	auto& impl = *this->Implementations[this->SelectedLocation];
	impl.Ui.Files->clearSelection();
}

//-----------------------------------------------------------------------------
void pqFileDialog::onClickedFile(const QModelIndex& index)
{

}

//-----------------------------------------------------------------------------
void pqFileDialog::onActivateFavorite(const QModelIndex& index)
{

}

//-----------------------------------------------------------------------------
void pqFileDialog::onActivateLocation(const QModelIndex& index)
{

}

//-----------------------------------------------------------------------------
void pqFileDialog::onActivateRecent(const QModelIndex& index)
{

}

//-----------------------------------------------------------------------------
void pqFileDialog::onDoubleClickFile(const QModelIndex&)
{
	auto& impl = *this->Implementations[this->SelectedLocation];
	QScopedValueRollback<bool> setter(impl.InDoubleClickHandler, true);
	this->accept();
}

//-----------------------------------------------------------------------------
void pqFileDialog::onShowHiddenFiles(const bool& hidden)
{

}

//-----------------------------------------------------------------------------
void pqFileDialog::onShowDetailToggled(bool show)
{

}

//-----------------------------------------------------------------------------
void pqFileDialog::onGroupFilesToggled(bool group)
{

}

//-----------------------------------------------------------------------------
void pqFileDialog::setShowHidden(const bool& hidden)
{
	this->onShowHiddenFiles(hidden);
}

//-----------------------------------------------------------------------------
bool pqFileDialog::getShowHidden()
{
	return false;
}

//-----------------------------------------------------------------------------
void pqFileDialog::onTextEdited(const QString& str)
{
	auto& impl = *this->Implementations[this->SelectedLocation];
	impl.Ui.Favorites->clearSelection();

	// really important to block signals so that the clearSelection
	// doesn't cause a signal to be fired that calls fileSelectionChanged
	impl.Ui.Files->blockSignals(true);
	impl.Ui.Files->clearSelection();
	if (str.size() > 0)
	{
		// convert the typed information to be impl.FileNames
		impl.FileNames = str.split(impl.FileNamesSeperator, Qt::SkipEmptyParts);
	}
	else
	{
		impl.FileNames.clear();
	}
	impl.Ui.Files->blockSignals(false);
	this->updateButtonStates(this->SelectedLocation);
}

//-----------------------------------------------------------------------------
QString pqFileDialog::fixFileExtension(const QString& filename, const QString& filter)
{
	auto& impl = *this->Implementations[this->SelectedLocation];
	// Add missing extension if necessary
	QFileInfo fileInfo(filename);
	QString ext = fileInfo.completeSuffix();
	QString extensionWildcard = GetWildCardsFromFilter(filter).first();
	QString wantedExtension = extensionWildcard.mid(extensionWildcard.indexOf('.') + 1);

	if (!ext.isEmpty())
	{
		// Ensure that the extension the user added is indeed of one the supported
		// types. (BUG #7634).
		QStringList wildCards;
		Q_FOREACH(QString curfilter, impl.Filters)
		{
			wildCards += ::GetWildCardsFromFilter(curfilter);
		}
		bool pass = false;
		Q_FOREACH(QString wildcard, wildCards)
		{
			if (wildcard.indexOf('.') != -1)
			{
				// we only need to validate the extension, not the filename.
				wildcard = QString("*.%1").arg(wildcard.mid(wildcard.indexOf('.') + 1));
				QRegExp regEx = QRegExp(wildcard, Qt::CaseInsensitive, QRegExp::Wildcard);
				if (regEx.exactMatch(fileInfo.fileName()))
				{
					pass = true;
					break;
				}
			}
			else
			{
				// we have a filter which does not specify any rule for the extension.
				// In that case, just assume the extension matched.
				pass = true;
				break;
			}
		}
		if (!pass)
		{
			// force adding of the wantedExtension.
			ext = QString();
		}
	}

	QString fixedFilename = filename;
	if (ext.isEmpty() && !wantedExtension.isEmpty() && wantedExtension != "*")
	{
		if (fixedFilename.at(fixedFilename.size() - 1) != '.')
		{
			fixedFilename += ".";
		}
		fixedFilename += wantedExtension;
	}
	return fixedFilename;
}

//-----------------------------------------------------------------------------
bool pqFileDialog::acceptInternal(const QStringList& selected_files)
{
	if (selected_files.empty())
	{
		return false;
	}
	return false;
}

//-----------------------------------------------------------------------------
void pqFileDialog::fileSelectionChanged()
{

}

//-----------------------------------------------------------------------------
bool pqFileDialog::selectFile(const QString& f)
{

	return true;
}

//-----------------------------------------------------------------------------
void pqFileDialog::showEvent(QShowEvent* _showEvent)
{
	auto& impl = *this->Implementations[this->SelectedLocation];
	QDialog::showEvent(_showEvent);
	// Qt sets the default keyboard focus to the last item in the tab order
	// which is determined by the creation order. This means that we have
	// to explicitly state that the line edit has the focus on showing no
	// matter the tab order
	impl.Ui.EntityName->setFocus(Qt::OtherFocusReason);
}

//-----------------------------------------------------------------------------
QPair<QString, unsigned int> pqFileDialog::getSaveFileNameAndLocation(
	QWidget* parentWdg, const QString& title, const QString& directory, const QString& filter,
	bool supportGroupFiles, bool onlyBrowseRemotely)
{
	pqFileDialog fileDialog(
		parentWdg, title, directory, filter, supportGroupFiles, onlyBrowseRemotely);
	fileDialog.setObjectName("FileOpenDialog");
	fileDialog.setFileMode(pqFileDialog::AnyFile);
	if (fileDialog.exec() == QDialog::Accepted)
	{
		return { fileDialog.getSelectedFiles()[0], fileDialog.getSelectedLocation() };
	}
	return { QString(), fileDialog.getSelectedLocation() };
}

//-----------------------------------------------------------------------------
void pqFileDialog::saveState(unsigned int location)
{

}

//-----------------------------------------------------------------------------
void pqFileDialog::saveState()
{

}

//-----------------------------------------------------------------------------
void pqFileDialog::restoreState(unsigned int location)
{

}

//-----------------------------------------------------------------------------
void pqFileDialog::restoreState()
{

}

//-----------------------------------------------------------------------------
void pqFileDialog::updateButtonStates(unsigned int location)
{

}
