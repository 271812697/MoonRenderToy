<<<<<<< HEAD
#include <algorithm>
	====== =
	ï» ? include <algorithm>
	>>>>>> > 55bf1f75852debbf88cca55155cea5d07b047be7
#include "QDirectoryModel.h"
#include <filesystem>
	using namespace std;
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
	QDirectoryModel::QDirectoryModel()
		: QStandardItemModel()
	{
	}

	void QDirectoryModel::setRootPath(const RootPathArray& rootPaths, const char* extFilter, QTreeView* treeView, QSortFilterProxyModel* proxy)
	{
		m_rootPaths = rootPaths;
		m_exts.push_back("none");

		m_treeView = treeView;
		m_proxy = proxy;
		connect(m_treeView, SIGNAL(expanded(const QModelIndex&)), this, SLOT(OnExpandedFilter(const QModelIndex&)));
		connect(m_treeView, SIGNAL(collapsed(const QModelIndex&)), this, SLOT(OnExpandedFilter(const QModelIndex&)));
		connect(m_treeView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(onSelectedFile(const QModelIndex&)));
		connect(m_treeView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(OnEditFile(const QModelIndex&)));
	}

	bool QDirectoryModel::IsSupportExt(const string& ext)
	{
		return true;
	}

	void QDirectoryModel::Clean()
	{
		this->clear();
		m_dirItems.clear();
		m_activeItem = nullptr;
	}

	void QDirectoryModel::setCurrentSelect(const char* dir)
	{
		// find item
		QStandardItem* lastItem = NULL;
		for (size_t i = 0; i < m_dirItems.size(); i++)
		{
			QStandardItem* item = m_dirItems[i];
			if (item && item->data(Qt::UserRole).toString() == dir)
			{
				lastItem = item;
				break;
			}
		}

		if (lastItem)
		{
			// set current select
			QModelIndex lastIdx = indexFromItem(lastItem);
			m_treeView->setCurrentIndex(lastIdx);
			onSelectedFile(lastIdx);

			// expand parent
			QStandardItem* parentItem = lastItem->parent();
			while (parentItem)
			{
				QModelIndex parentIdx = indexFromItem(parentItem);
				m_treeView->expand(parentIdx);
				parentItem = parentItem->parent();
			}
			// expand
			m_treeView->expand(lastIdx);
		}
	}

	void QDirectoryModel::Refresh(bool volatile* interrupt)
	{
		for (RootPath& root : m_rootPaths)
		{
			if (std::filesystem::is_directory(root.m_path))
			{
				if (NULL != interrupt && *interrupt)
					return;
				std::string pathName = root.m_path.c_str();
				QStandardItem* rootItem = new QStandardItem;
				rootItem->setText(root.m_display.c_str());
				rootItem->setIcon(m_iconMaps["root"]);
				rootItem->setData(std::filesystem::directory_entry(root.m_path).path().generic_string().c_str(), Qt::UserRole);
				invisibleRootItem()->setChild(invisibleRootItem()->rowCount(), 0, rootItem);
				m_dirItems.emplace_back(rootItem);
				RecursiveDir(root.m_path, rootItem, interrupt);
				// expand root index
				if (root.m_expand)
					m_treeView->expand(rootItem->index());
			}
		}
	}

	QIcon QDirectoryModel::getFileIcon(const char* fullPath)
	{
		std::string fileExt = GetFileExt(fullPath);
		if (fileExt == "png")
		{
			QPixmap pixmap(fullPath);
			return QIcon(pixmap.scaled(QSize(64, 64)));
		}
		return m_iconMaps[fileExt.c_str()];
	}

	void QDirectoryModel::RecursiveDir(const string& dir, QStandardItem* parentItem, volatile bool* interrupt)
	{
		vector<QStandardItem*> dirItems;
		vector<QStandardItem*> fileItems;
		std::filesystem::directory_entry p_directory(dir);
		for (auto& item : std::filesystem::directory_iterator(p_directory)) {
			if (NULL != interrupt && *interrupt)
				return;
			if (item.is_directory()) {
				std::string dirName = GetElementName(item.path().string());
				QStandardItem* childItem = new QStandardItem;
				childItem->setText(dirName.c_str());
				childItem->setIcon(m_iconMaps["filter"]);
				childItem->setData(item.path().generic_string().c_str(), Qt::UserRole);
				dirItems.emplace_back(childItem);
				m_dirItems.emplace_back(childItem);
				RecursiveDir(item.path().string(), childItem, interrupt);
			}
			else
			{
				std::string fileExt = GetFileExt(item.path().string());
				if (IsSupportExt(fileExt))
				{
					std::string pureFileName = item.path().filename().string();
					QStandardItem* childItem = new QStandardItem;
					childItem->setText(pureFileName.c_str());
					childItem->setIcon(getFileIcon(item.path().string().c_str()));
					childItem->setData(item.path().generic_string().c_str(), Qt::UserRole);

					fileItems.emplace_back(childItem);
				}
			}
		}
		int tRow = 0;
		// insert folder first
		{
			for (size_t i = 0; i < dirItems.size(); i++)
			{
				parentItem->setChild(tRow, 0, dirItems[i]);
				tRow++;
			}
			// then input file after
			for (size_t i = 0; i < fileItems.size(); i++)
			{
				parentItem->setChild(tRow, 0, fileItems[i]);
				tRow++;
			}
		}
	}

	void QDirectoryModel::OnExpandedFilter(const QModelIndex& pIndex)
	{
		if (m_treeView->isExpanded(pIndex))
			itemFromIndex(pIndex)->setIcon(m_iconMaps["filterexpend"]);
		else
			itemFromIndex(pIndex)->setIcon(m_iconMaps["filter"]);
	}

	QString QDirectoryModel::getFileUnderMousePos(const QPoint& pos)
	{
		QModelIndex index = m_treeView->indexAt(pos);
		QString filePath = m_proxy ? m_proxy->data(index, Qt::UserRole).toString() : this->data(index, Qt::UserRole).toString();

		return filePath;
	}

	void QDirectoryModel::onSelectedFile(const QModelIndex& pIndex)
	{
		m_currentSelect = pIndex;
		QString filePath = m_proxy ? m_proxy->data(pIndex, Qt::UserRole).toString() : this->data(pIndex, Qt::UserRole).toString();
		<<<<<<< HEAD
			// È¡Ïû¼¤»î // ´ÖÌåÏÔÊ¾
			====== =

			// å–æ¶ˆæ¿€æ´?// ç²—ä½“æ˜¾ç¤º
			>>>>>> > 55bf1f75852debbf88cca55155cea5d07b047be7
			if (m_activeItem)
				m_activeItem->setFont(m_treeView->font());

		// è®¾ç½®æ¿€æ´?
		m_activeItem = itemFromIndex(pIndex);
		if (m_activeItem)
		{
			// ç•Œé¢å“åº”
			QFont font = m_treeView->font();
			font.setBold(true);
			m_activeItem->setFont(font);
		}
		emit FileSelected(filePath.toStdString().c_str());
	}

	void QDirectoryModel::OnEditFile(const QModelIndex& pIndex)
	{
		QString filePath = m_proxy ? m_proxy->data(pIndex, Qt::UserRole).toString() : this->data(pIndex, Qt::UserRole).toString();

		// å–æ¶ˆæ¿€æ´?// ç²—ä½“æ˜¾ç¤º
		if (m_activeItem)
			m_activeItem->setFont(m_treeView->font());

		// è®¾ç½®æ¿€æ´?
		m_activeItem = itemFromIndex(pIndex);
		if (m_activeItem)
		{
			// ç•Œé¢å“åº”
			QFont font = m_treeView->font();
			font.setBold(true);

			m_activeItem->setFont(font);
		}

		emit FileEdit(filePath.toStdString().c_str());
	}
}


