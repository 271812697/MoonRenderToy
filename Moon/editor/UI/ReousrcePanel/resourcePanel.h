#pragma once

#include <QDockWidget>
namespace MOON {


	class ResPanel : public QDockWidget
	{
		Q_OBJECT

	public:
		ResPanel(QWidget* parent = 0);
		~ResPanel();

		// instance
		static ResPanel* instance();

		// call when open project
		void onOpenProject();

		// recover edit settings
		void recoverEditSettings();

	public slots:
		// when select dir
		void onSelectDir(const char* dir);
		void onSelectFile(const char* pathName);

		// reselect current dir
		void reslectCurrentDir();

		// click res
		void onClickedPreviewRes(const char* res);
		void onDoubleClickedPreviewRes(const char* res);

		// node tree widget show menu
		void showMenu(const QPoint& point);

		// show current dir
		void showInExporer();

		// new folder
		void newFolder();

		// res
		void onCreateRes();
		void onImportRes();

		// rename res
		void onRenameRes();

		// delete res
		void onDeleteRes();

		// duplicate res
		void onDuplicateRes();

		// copy res path
		void onCopyResPath();

		// on renamed res
		void onRenamedRes(const QString src, const QString dest);

		// on switch view type
		void onSwitchResVeiwType();

		// on search text changed
		void onSearchTextChanged();

		// file system changed (QFileSystemWatcher)
		void onWatchFileChanged(const QString& file);
		void onWatchFileDirChanged(const QString& dir);

	private:
		// get unique file name
		//bool getUniqueNewResSavePath(Echo::String& outNewPath, const Echo::String& className, const Echo::String& currentDir);

		// add import action to menu
		//void addImporterActionToMenu(QMenu* menu, const Echo::String& className);

	protected:
		// re implement reiszeEvent function
		virtual void resizeEvent(QResizeEvent* e);

	private:
		class ResPanelInternal;
		ResPanelInternal* internal;

	};
}



