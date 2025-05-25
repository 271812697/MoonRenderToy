
#include "QPreviewHelper.h"
#include <QFileInfo>
#include <QDateTime>
#include <filesystem>


namespace MOON
{
	static std::string GetFileExt(const std::string& p_path)
	{
		std::string result;

		for (auto it = p_path.rbegin(); it != p_path.rend() && *it != '.'; ++it)
			result += *it;

		std::reverse(result.begin(), result.end());

		return result;
	}
	static std::string GetElementName(const std::string& p_path)
	{
		std::string result;

		std::string path = p_path;
		if (!path.empty() && path.back() == '\\')
			path.pop_back();

		for (auto it = path.rbegin(); it != path.rend() && *it != '\\' && *it != '/'; ++it)
			result += *it;

		std::reverse(result.begin(), result.end());

		return result;
	}
	QPreviewHelper::QPreviewHelper(QListView* view)
		: m_listView(view)
	{
		m_listModel = new QResListModel(m_listView);

		m_listProxyModel = new QSortFilterProxyModel(m_listView);
		m_listProxyModel->setSourceModel(m_listModel);
		m_listProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
		m_listProxyModel->setFilterKeyColumn(0);

		m_listView->setModel(m_listProxyModel);
		m_listView->setAttribute(Qt::WA_MacShowFocusRect, 0);

		QObject::connect(m_listView, SIGNAL(clicked(QModelIndex)), this, SLOT(onClicked(QModelIndex)));
		QObject::connect(m_listView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClicked(QModelIndex)));
		QObject::connect(m_listModel, &QStandardItemModel::itemChanged, this, &QPreviewHelper::renameRes);

		setUseIconMode();
	}



	void QPreviewHelper::setPath(const std::string& filePath, const char* exts, bool includePreDir)
	{
		// include pre directory
		if (includePreDir)
		{
			addItem((filePath + "../").c_str());
		}
		std::filesystem::path p(filePath);

		std::filesystem::directory_entry p_directory(filePath);
		if (p_directory.is_directory()) {
			for (auto& item : std::filesystem::directory_iterator(p_directory)) {
				if (item.is_directory()) {
					addItem(item.path().string().c_str());
				}
			}
			for (auto& item : std::filesystem::directory_iterator(p_directory)) {
				if (!item.is_directory()) {
					addItem(item.path().string().c_str());
				}
			}
		}
		else
		{
			addItem(p_directory.path().string().c_str());
		}

	}

	void QPreviewHelper::selectFile(const std::string& fileName)
	{
		QList<QStandardItem*> items = m_listModel->findItems(fileName.c_str());
		for (QStandardItem* item : items)
		{
			m_listView->setCurrentIndex(m_listProxyModel->mapFromSource(item->index()));
		}
	}

	void QPreviewHelper::setFilterPattern(const char* pattern)
	{
		QRegExp regExp(pattern, Qt::CaseInsensitive);
		m_listProxyModel->setFilterRegExp(regExp);
	}

	void QPreviewHelper::addItem(const char* filePath, const char* displayText)
	{
		std::vector<QStandardItem*> results;
		createItem(filePath, displayText, results);
		for (QStandardItem* item : results)
		{
			m_listModel->appendRow(item);
		}
	}

	void QPreviewHelper::createItem(const char* filePath, const char* displayText, std::vector<QStandardItem*>& results)
	{
		QStandardItem* item = nullptr;
		std::filesystem::directory_entry p_directory(filePath);

		if (p_directory.is_directory())
		{
			std::string folderName = displayText ? displayText : GetElementName(filePath);
			item = new QStandardItem(QIcon(":/icon/Icon/root.png"), folderName.c_str());
		}
		else
		{
			std::string fileName = displayText ? displayText : GetElementName(filePath);
			item = new QStandardItem(getFileIcon(filePath), fileName.c_str());
		}

		if (item)
		{
			item->setData(filePath, Qt::UserRole);
			item->setSizeHint(QSize(m_itemWidth, m_itemHeight));
			addToolTips(item, filePath);
			results.emplace_back(item);
		}
	}

	void QPreviewHelper::addToolTips(QStandardItem* item, const std::string& fullPath)
	{

		std::string fileName = GetElementName(fullPath);
		QFileInfo fileInfo(fullPath.c_str());
		qint64    fileSize = fileInfo.size();
		std::string lastModify = fileInfo.lastModified().toString("yyyy/mm/dd hh:mm:ss").toStdString().c_str();

		std::string tips;
		tips += "Name : " + fileName + "\n";
		tips += "Size : " + std::to_string(std::max<int>(fileSize / 1024, 1)) + "kb\n";// Echo::StringUtil::Format("%d kb\n", std::max<int>(fileSize / 1024, 1));
		tips += "Modify : " + lastModify + "\n";
		tips += "Path : " + fullPath;
		item->setToolTip(tips.c_str());
	}



	QIcon QPreviewHelper::getFileIcon(const char* fullPath)
	{
		return QIcon(":/icon/Icon/file/file.png");
	}

	void QPreviewHelper::clear()
	{
		m_listModel->clear();
	}

	void QPreviewHelper::setUseIconMode()
	{
		m_iconSize = 64;
		m_listView->setIconSize(QSize(m_iconSize, m_iconSize));
		m_listView->setResizeMode(QListView::Adjust);
		m_listView->setViewMode(QListView::IconMode);
		m_listView->setMovement(QListView::Static);
		m_listView->setFlow(QListView::LeftToRight);
		m_listView->setSpacing(5);
		setItemSizeHint(68, 86);
	}

	void QPreviewHelper::setUseListMode()
	{
		m_iconSize = 30;
		m_listView->setIconSize(QSize(m_iconSize, m_iconSize));
		m_listView->setResizeMode(QListView::Adjust);
		m_listView->setViewMode(QListView::ListMode);
		m_listView->setMovement(QListView::Static);
		m_listView->setFlow(QListView::TopToBottom);
		m_listView->setSpacing(0);
		setItemSizeHint(512, 30);
	}

	void QPreviewHelper::setItemSizeHint(int width, int height)
	{
		m_itemWidth = width;
		m_itemHeight = height;
		for (int i = 0; i < m_listModel->rowCount(); i++)
		{
			QStandardItem* item = m_listModel->item(i, 0);
			if (item)
				item->setSizeHint(QSize(m_itemWidth, m_itemHeight));
		}
	}

	// is support this ext
	bool QPreviewHelper::isSupportExt(const std::string& file)
	{
		if (m_supportExts.empty())
			return true;
		return true;
	}

	// when resize list view, modify spacing
	void QPreviewHelper::onListViewResize()
	{
		if (m_listView->viewMode() == QListView::IconMode)
		{
			float listViewWidth = m_listView->geometry().width() - 26;
			float iconSizeWidth = m_listView->iconSize().width();
			int numberIcons = std::max<int>(listViewWidth / iconSizeWidth, 1);
			int spacing = std::max<int>(listViewWidth - numberIcons * iconSizeWidth, 0) / numberIcons / 2;

			m_listView->setSpacing(std::max<int>(spacing, 0));
		}
	}

	// item at
	QStandardItem* QPreviewHelper::itemAt(const QPoint& pos)
	{
		QModelIndex proxyIndex = m_listView->indexAt(pos);

		const QModelIndex index = m_listProxyModel->mapToSource(proxyIndex);
		return m_listModel->itemFromIndex(index);
	}

	// edit item
	void QPreviewHelper::editItem(QStandardItem* item)
	{
		if (item)
		{
			m_listView->edit(m_listProxyModel->mapFromSource(item->index()));
		}
	}

	// clicked resource
	void QPreviewHelper::onClicked(const QModelIndex& pIndex)
	{
		std::string resPath = m_listProxyModel ? m_listProxyModel->data(pIndex, Qt::UserRole).toString().toStdString().c_str() : m_listModel->data(pIndex, Qt::UserRole).toString().toStdString().c_str();


		emit clickedRes(resPath.c_str());
	}

	// double clicked resource
	void QPreviewHelper::onDoubleClicked(const QModelIndex& pIndex)
	{
		std::string resPath = m_listProxyModel ? m_listProxyModel->data(pIndex, Qt::UserRole).toString().toStdString().c_str() : m_listModel->data(pIndex, Qt::UserRole).toString().toStdString().c_str();

		emit doubleClickedRes(resPath.c_str());
	}

	// rename res
	void QPreviewHelper::renameRes(QStandardItem* item)
	{
		if (item)
		{
			std::string preFilePathName = item->data(Qt::UserRole).toString().toStdString().c_str();
			std::string currentText = item->text().toStdString().c_str();
			std::filesystem::directory_entry p_directory(preFilePathName);
			std::string folderName = GetElementName(preFilePathName);;
			if (currentText != folderName)
			{
				std::string newPath = p_directory.path().parent_path().string() + currentText;

				std::filesystem::rename(preFilePathName, newPath);

				item->setData(newPath.c_str(), Qt::UserRole);

				emit renamedRes(preFilePathName.c_str(), newPath.c_str());
			}

		}
	}
}


