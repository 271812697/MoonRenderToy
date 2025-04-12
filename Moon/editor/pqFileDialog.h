#pragma once
#include <QDialog>
#include <QMap>
#include <QPointer>
#include <QStringList>

class QModelIndex;
class QPoint;
class QShowEvent;
//class pqServer;


class  pqFileDialog : public QDialog
{
	typedef QDialog Superclass;
	Q_OBJECT
public:

	enum FileMode
	{
		AnyFile,
		ExistingFile,
		ExistingFiles,
		Directory,
		ExistingFilesAndDirectories
	};

	pqFileDialog(QWidget* parent, const QString& title = QString(),
		const QString& directory = QString(), const QString& filter = QString(),
		bool supportGroupFiles = true, bool onlyBrowseRemotely = true);
	~pqFileDialog() override;


	void setFileMode(FileMode, unsigned int);
	void setFileMode(FileMode);

	void setRecentlyUsedExtension(const QString& fileExtension, unsigned int location);
	void setRecentlyUsedExtension(const QString& fileExtension);
	///@}


	QStringList getSelectedFiles(int index = 0);


	QList<QStringList> getAllSelectedFiles();


	int getSelectedFilterIndex();


	void accept() override;


	bool selectFile(const QString&);


	void setShowHidden(const bool& hidden);


	bool getShowHidden();


	unsigned int getSelectedLocation() const { return this->SelectedLocation; }


	static QString getSaveFileName(QWidget* parentWdg,
		const QString& title = QString(), const QString& directory = QString(),
		const QString& filter = QString())
	{
		const QPair<QString, unsigned int> result =
			pqFileDialog::getSaveFileNameAndLocation(parentWdg, title, directory, filter);
		return result.first;
	}
	static QPair<QString, unsigned int> getSaveFileNameAndLocation(
		QWidget* parentWdg, const QString& title = QString(), const QString& directory = QString(),
		const QString& filter = QString(), bool supportGroupFiles = false,
		bool onlyBrowseRemotely = true);
Q_SIGNALS:
	void filesSelected(const QList<QStringList>&);
	void filesSelected(const QStringList&);
	void fileAccepted(const QString&);

protected:
	bool acceptExistingFiles();
	bool acceptDefault(const bool& checkForGrouping);

	QStringList buildFileGroup(const QString& filename);

	void showEvent(QShowEvent* showEvent) override;

private Q_SLOTS:
	void onLocationChanged(int fs);
	void onModelReset();
	void onNavigate(const QString & = QString());
	void onNavigateUp();
	void onNavigateBack();
	void onNavigateForward();
	void onNavigateDown(const QModelIndex&);
	void onFilterChange(const QString&);

	void onClickedRecent(const QModelIndex&);
	void onClickedFavorite(const QModelIndex&);
	void onClickedFile(const QModelIndex&);

	void onActivateFavorite(const QModelIndex&);
	void onActivateLocation(const QModelIndex&);
	void onActivateRecent(const QModelIndex&);
	void onDoubleClickFile(const QModelIndex&);

	void onTextEdited(const QString&);

	void onShowHiddenFiles(const bool& hide);

	void onShowDetailToggled(bool show);

	void onGroupFilesToggled(bool group);

	void fileSelectionChanged();

	void onContextMenuRequested(const QPoint& pos);

	void onFavoritesContextMenuRequested(const QPoint& pos);

	void AddDirectoryToFavorites(QString const&);
	void RemoveDirectoryFromFavorites(QString const&);
	void FilterDirectoryFromFavorites(const QString& filter);

	void onAddCurrentDirectoryToFavorites();
	void onRemoveSelectedDirectoriesFromFavorites();
	void onResetFavoritesToSystemDefault();

	void onCreateNewFolder();

	void addToFilesSelected(const QStringList&);


	void emitFilesSelectionDone();


	void updateButtonStates(unsigned int fileSystem);

private: // NOLINT(readability-redundant-access-specifiers)
	pqFileDialog(const pqFileDialog&);
	pqFileDialog& operator=(const pqFileDialog&);

	class pqImplementation;
	QMap<unsigned int, QPointer<pqImplementation>> Implementations;
	unsigned int SelectedLocation;


	const QString StartDirectory;
	const QString NameFilter;
	bool SupportsGroupFiles;
	unsigned int DefaultLocation;
	QString RecentlyUsedExtension;

	void addImplementation(unsigned int location);

	bool acceptInternal(const QStringList& selected_files);
	QString fixFileExtension(const QString& filename, const QString& filter);


	void saveState(unsigned int fileSystem);
	void saveState();

	void restoreState(unsigned int fileSystem);
	void restoreState();
};