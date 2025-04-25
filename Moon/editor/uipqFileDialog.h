#pragma once
#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

class pqFileComboBox : public QComboBox
{
public:
	pqFileComboBox(QWidget* p)
		: QComboBox(p)
	{
	}
	void showPopup() override
	{
		QWidget* container = this->view()->parentWidget();
		container->setMaximumWidth(this->width());
		QComboBox::showPopup();
	}
};
QT_BEGIN_NAMESPACE

class Ui_pqFileDialog
{
public:
	QGridLayout* mainGridLayout;
	QWidget* mainWidget;
	QGridLayout* gridLayout;
	QHBoxLayout* hboxLayout;
	QLabel* label_3;
	QComboBox* Parents;
	QToolButton* NavigateBack;
	QToolButton* NavigateForward;
	QToolButton* NavigateUp;
	QToolButton* CreateFolder;
	QToolButton* ShowDetail;
	QToolButton* GroupFiles;
	QSplitter* mainSplitter;
	QSplitter* splitter;
	QWidget* favoritesLayoutWidget;
	QVBoxLayout* favoritesVerticalLayout;
	QHBoxLayout* favoritesHorizontalLayout;
	QLabel* favoritesLabel;
	QPushButton* AddCurrentDirectoryToFavorites;
	QPushButton* ResetFavortiesToSystemDefault;
	QSpacerItem* verticalSpacer;
	QLineEdit* favoritesSearchBar;
	QListView* Favorites;
	QWidget* locationsLayoutWidget;
	QVBoxLayout* locationsVerticalLayout;
	QLabel* locationsLabel;
	QListView* Locations;
	QWidget* recentLayoutWidget;
	QVBoxLayout* recentVerticalLayout;
	QHBoxLayout* recentHorizontalLayout;
	QLabel* recentLabel;
	QListView* Recent;
	QWidget* layoutWidget;
	QVBoxLayout* verticalLayout;
	QTreeView* Files;
	QGridLayout* gridLayout1;
	QLabel* EntityNameLabel;
	QLineEdit* EntityName;
	QLabel* EntityTypeLabel;
	QPushButton* OK;
	QPushButton* Cancel;
	QPushButton* Navigate;
	pqFileComboBox* EntityType;

	void setupUi(QWidget* pqFileDialog)
	{
		if (pqFileDialog->objectName().isEmpty())
			pqFileDialog->setObjectName(QString::fromUtf8("pqFileDialog"));
		pqFileDialog->resize(683, 402);
		mainGridLayout = new QGridLayout(pqFileDialog);
		mainGridLayout->setObjectName(QString::fromUtf8("mainGridLayout"));
		mainWidget = new QWidget(pqFileDialog);
		mainWidget->setObjectName(QString::fromUtf8("mainWidget"));
		mainWidget->setGeometry(QRect(0, 0, 683, 402));
		gridLayout = new QGridLayout(mainWidget);
		gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
		gridLayout->setContentsMargins(0, 0, 0, 0);
		hboxLayout = new QHBoxLayout();
		hboxLayout->setSpacing(6);
		hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
		hboxLayout->setContentsMargins(0, 0, 0, 0);
		label_3 = new QLabel(mainWidget);
		label_3->setObjectName(QString::fromUtf8("label_3"));

		hboxLayout->addWidget(label_3);

		Parents = new QComboBox(mainWidget);
		Parents->setObjectName(QString::fromUtf8("Parents"));
		QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		sizePolicy.setHorizontalStretch(0);
		sizePolicy.setVerticalStretch(0);
		sizePolicy.setHeightForWidth(Parents->sizePolicy().hasHeightForWidth());
		Parents->setSizePolicy(sizePolicy);
		Parents->setEditable(true);

		hboxLayout->addWidget(Parents);

		NavigateBack = new QToolButton(mainWidget);
		NavigateBack->setObjectName(QString::fromUtf8("NavigateBack"));
		NavigateBack->setAutoRaise(true);

		hboxLayout->addWidget(NavigateBack);

		NavigateForward = new QToolButton(mainWidget);
		NavigateForward->setObjectName(QString::fromUtf8("NavigateForward"));
		NavigateForward->setAutoRaise(true);

		hboxLayout->addWidget(NavigateForward);

		NavigateUp = new QToolButton(mainWidget);
		NavigateUp->setObjectName(QString::fromUtf8("NavigateUp"));
		NavigateUp->setAutoRaise(true);

		hboxLayout->addWidget(NavigateUp);

		CreateFolder = new QToolButton(mainWidget);
		CreateFolder->setObjectName(QString::fromUtf8("CreateFolder"));
		CreateFolder->setAutoRaise(true);

		hboxLayout->addWidget(CreateFolder);

		ShowDetail = new QToolButton(mainWidget);
		ShowDetail->setObjectName(QString::fromUtf8("ShowDetail"));
		ShowDetail->setCheckable(true);

		hboxLayout->addWidget(ShowDetail);

		GroupFiles = new QToolButton(mainWidget);
		GroupFiles->setObjectName(QString::fromUtf8("GroupFiles"));
		GroupFiles->setCheckable(true);
		QIcon icon;
		icon.addFile(QString::fromUtf8(":/widgets/icons/pqGroupFiles.svg"), QSize(), QIcon::Normal, QIcon::Off);
		GroupFiles->setIcon(icon);

		hboxLayout->addWidget(GroupFiles);

		hboxLayout->setStretch(1, 1);

		gridLayout->addLayout(hboxLayout, 0, 0, 1, 1);

		mainSplitter = new QSplitter(mainWidget);
		mainSplitter->setObjectName(QString::fromUtf8("mainSplitter"));
		mainSplitter->setOrientation(Qt::Horizontal);
		splitter = new QSplitter(mainSplitter);
		splitter->setObjectName(QString::fromUtf8("splitter"));
		QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Expanding);
		sizePolicy1.setHorizontalStretch(0);
		sizePolicy1.setVerticalStretch(0);
		sizePolicy1.setHeightForWidth(splitter->sizePolicy().hasHeightForWidth());
		splitter->setSizePolicy(sizePolicy1);
		splitter->setOrientation(Qt::Vertical);
		favoritesLayoutWidget = new QWidget(splitter);
		favoritesLayoutWidget->setObjectName(QString::fromUtf8("favoritesLayoutWidget"));
		QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Expanding);
		sizePolicy2.setHorizontalStretch(1);
		sizePolicy2.setVerticalStretch(1);
		sizePolicy2.setHeightForWidth(favoritesLayoutWidget->sizePolicy().hasHeightForWidth());
		favoritesLayoutWidget->setSizePolicy(sizePolicy2);
		favoritesVerticalLayout = new QVBoxLayout(favoritesLayoutWidget);
		favoritesVerticalLayout->setSpacing(0);
		favoritesVerticalLayout->setObjectName(QString::fromUtf8("favoritesVerticalLayout"));
		favoritesVerticalLayout->setContentsMargins(0, 0, 0, 0);
		favoritesHorizontalLayout = new QHBoxLayout();
		favoritesHorizontalLayout->setSpacing(5);
		favoritesHorizontalLayout->setObjectName(QString::fromUtf8("favoritesHorizontalLayout"));
		favoritesLabel = new QLabel(favoritesLayoutWidget);
		favoritesLabel->setObjectName(QString::fromUtf8("favoritesLabel"));

		favoritesHorizontalLayout->addWidget(favoritesLabel);

		AddCurrentDirectoryToFavorites = new QPushButton(favoritesLayoutWidget);
		AddCurrentDirectoryToFavorites->setObjectName(QString::fromUtf8("AddCurrentDirectoryToFavorites"));
		QSizePolicy sizePolicy3(QSizePolicy::Fixed, QSizePolicy::Fixed);
		sizePolicy3.setHorizontalStretch(0);
		sizePolicy3.setVerticalStretch(0);
		sizePolicy3.setHeightForWidth(AddCurrentDirectoryToFavorites->sizePolicy().hasHeightForWidth());
		AddCurrentDirectoryToFavorites->setSizePolicy(sizePolicy3);
		QIcon icon1;
		QString iconThemeName = QString::fromUtf8("list-add");
		if (QIcon::hasThemeIcon(iconThemeName)) {
			icon1 = QIcon::fromTheme(iconThemeName);
		}
		else {
			icon1.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
		}
		AddCurrentDirectoryToFavorites->setIcon(icon1);

		favoritesHorizontalLayout->addWidget(AddCurrentDirectoryToFavorites);

		ResetFavortiesToSystemDefault = new QPushButton(favoritesLayoutWidget);
		ResetFavortiesToSystemDefault->setObjectName(QString::fromUtf8("ResetFavortiesToSystemDefault"));
		sizePolicy3.setHeightForWidth(ResetFavortiesToSystemDefault->sizePolicy().hasHeightForWidth());
		ResetFavortiesToSystemDefault->setSizePolicy(sizePolicy3);
		QIcon icon2;
		iconThemeName = QString::fromUtf8("view-refresh");
		if (QIcon::hasThemeIcon(iconThemeName)) {
			icon2 = QIcon::fromTheme(iconThemeName);
		}
		else {
			icon2.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
		}
		ResetFavortiesToSystemDefault->setIcon(icon2);

		favoritesHorizontalLayout->addWidget(ResetFavortiesToSystemDefault);


		favoritesVerticalLayout->addLayout(favoritesHorizontalLayout);

		verticalSpacer = new QSpacerItem(20, 6, QSizePolicy::Minimum, QSizePolicy::Minimum);

		favoritesVerticalLayout->addItem(verticalSpacer);

		favoritesSearchBar = new QLineEdit(favoritesLayoutWidget);
		favoritesSearchBar->setObjectName(QString::fromUtf8("favoritesSearchBar"));
		favoritesSearchBar->setMaximumSize(QSize(16777215, 18));
		QFont font;
		font.setPointSize(8);
		favoritesSearchBar->setFont(font);
		favoritesSearchBar->setClearButtonEnabled(true);

		favoritesVerticalLayout->addWidget(favoritesSearchBar);

		Favorites = new QListView(favoritesLayoutWidget);
		Favorites->setObjectName(QString::fromUtf8("Favorites"));
		QSizePolicy sizePolicy4(QSizePolicy::Preferred, QSizePolicy::Expanding);
		sizePolicy4.setHorizontalStretch(1);
		sizePolicy4.setVerticalStretch(0);
		sizePolicy4.setHeightForWidth(Favorites->sizePolicy().hasHeightForWidth());
		Favorites->setSizePolicy(sizePolicy4);
		Favorites->setMinimumSize(QSize(100, 0));
		Favorites->setSelectionMode(QAbstractItemView::NoSelection);
		Favorites->setSelectionBehavior(QAbstractItemView::SelectRows);

		favoritesVerticalLayout->addWidget(Favorites);

		splitter->addWidget(favoritesLayoutWidget);
		locationsLayoutWidget = new QWidget(splitter);
		locationsLayoutWidget->setObjectName(QString::fromUtf8("locationsLayoutWidget"));
		QSizePolicy sizePolicy5(QSizePolicy::Expanding, QSizePolicy::Expanding);
		sizePolicy5.setHorizontalStretch(1);
		sizePolicy5.setVerticalStretch(2);
		sizePolicy5.setHeightForWidth(locationsLayoutWidget->sizePolicy().hasHeightForWidth());
		locationsLayoutWidget->setSizePolicy(sizePolicy5);
		locationsVerticalLayout = new QVBoxLayout(locationsLayoutWidget);
		locationsVerticalLayout->setObjectName(QString::fromUtf8("locationsVerticalLayout"));
		locationsVerticalLayout->setContentsMargins(0, 0, 0, 0);
		locationsLabel = new QLabel(locationsLayoutWidget);
		locationsLabel->setObjectName(QString::fromUtf8("locationsLabel"));
		QSizePolicy sizePolicy6(QSizePolicy::Preferred, QSizePolicy::Minimum);
		sizePolicy6.setHorizontalStretch(0);
		sizePolicy6.setVerticalStretch(0);
		sizePolicy6.setHeightForWidth(locationsLabel->sizePolicy().hasHeightForWidth());
		locationsLabel->setSizePolicy(sizePolicy6);

		locationsVerticalLayout->addWidget(locationsLabel);

		Locations = new QListView(locationsLayoutWidget);
		Locations->setObjectName(QString::fromUtf8("Locations"));
		QSizePolicy sizePolicy7(QSizePolicy::Preferred, QSizePolicy::Expanding);
		sizePolicy7.setHorizontalStretch(1);
		sizePolicy7.setVerticalStretch(1);
		sizePolicy7.setHeightForWidth(Locations->sizePolicy().hasHeightForWidth());
		Locations->setSizePolicy(sizePolicy7);
		Locations->setMinimumSize(QSize(100, 0));
		Locations->setSelectionMode(QAbstractItemView::NoSelection);
		Locations->setSelectionBehavior(QAbstractItemView::SelectRows);

		locationsVerticalLayout->addWidget(Locations);

		splitter->addWidget(locationsLayoutWidget);
		recentLayoutWidget = new QWidget(splitter);
		recentLayoutWidget->setObjectName(QString::fromUtf8("recentLayoutWidget"));
		sizePolicy5.setHeightForWidth(recentLayoutWidget->sizePolicy().hasHeightForWidth());
		recentLayoutWidget->setSizePolicy(sizePolicy5);
		recentVerticalLayout = new QVBoxLayout(recentLayoutWidget);
		recentVerticalLayout->setObjectName(QString::fromUtf8("recentVerticalLayout"));
		recentVerticalLayout->setContentsMargins(0, 0, 0, 0);
		recentHorizontalLayout = new QHBoxLayout();
		recentHorizontalLayout->setObjectName(QString::fromUtf8("recentHorizontalLayout"));
		recentLabel = new QLabel(recentLayoutWidget);
		recentLabel->setObjectName(QString::fromUtf8("recentLabel"));

		recentHorizontalLayout->addWidget(recentLabel);


		recentVerticalLayout->addLayout(recentHorizontalLayout);

		Recent = new QListView(recentLayoutWidget);
		Recent->setObjectName(QString::fromUtf8("Recent"));
		sizePolicy7.setHeightForWidth(Recent->sizePolicy().hasHeightForWidth());
		Recent->setSizePolicy(sizePolicy7);
		Recent->setMinimumSize(QSize(100, 0));
		Recent->setSelectionMode(QAbstractItemView::NoSelection);
		Recent->setSelectionBehavior(QAbstractItemView::SelectRows);

		recentVerticalLayout->addWidget(Recent);

		splitter->addWidget(recentLayoutWidget);
		mainSplitter->addWidget(splitter);
		layoutWidget = new QWidget(mainSplitter);
		layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
		verticalLayout = new QVBoxLayout(layoutWidget);
		verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
		verticalLayout->setContentsMargins(0, 0, 0, 0);
		Files = new QTreeView(layoutWidget);
		Files->setObjectName(QString::fromUtf8("Files"));
		QSizePolicy sizePolicy8(QSizePolicy::Expanding, QSizePolicy::Expanding);
		sizePolicy8.setHorizontalStretch(2);
		sizePolicy8.setVerticalStretch(0);
		sizePolicy8.setHeightForWidth(Files->sizePolicy().hasHeightForWidth());
		Files->setSizePolicy(sizePolicy8);

		verticalLayout->addWidget(Files);

		gridLayout1 = new QGridLayout();
		gridLayout1->setSpacing(6);
		gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
		gridLayout1->setContentsMargins(0, 0, 0, 0);
		EntityNameLabel = new QLabel(layoutWidget);
		EntityNameLabel->setObjectName(QString::fromUtf8("EntityNameLabel"));

		gridLayout1->addWidget(EntityNameLabel, 0, 0, 1, 1);

		EntityName = new QLineEdit(layoutWidget);
		EntityName->setObjectName(QString::fromUtf8("EntityName"));

		gridLayout1->addWidget(EntityName, 0, 1, 1, 1);

		EntityTypeLabel = new QLabel(layoutWidget);
		EntityTypeLabel->setObjectName(QString::fromUtf8("EntityTypeLabel"));

		gridLayout1->addWidget(EntityTypeLabel, 1, 0, 1, 1);

		OK = new QPushButton(layoutWidget);
		OK->setObjectName(QString::fromUtf8("OK"));

		gridLayout1->addWidget(OK, 0, 3, 1, 1);

		Cancel = new QPushButton(layoutWidget);
		Cancel->setObjectName(QString::fromUtf8("Cancel"));

		gridLayout1->addWidget(Cancel, 1, 3, 1, 1);

		Navigate = new QPushButton(layoutWidget);
		Navigate->setObjectName(QString::fromUtf8("Navigate"));

		gridLayout1->addWidget(Navigate, 0, 2, 1, 1);

		EntityType = new pqFileComboBox(layoutWidget);
		EntityType->setObjectName(QString::fromUtf8("EntityType"));
		EntityType->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);

		gridLayout1->addWidget(EntityType, 1, 1, 1, 2);


		verticalLayout->addLayout(gridLayout1);

		mainSplitter->addWidget(layoutWidget);

		gridLayout->addWidget(mainSplitter, 1, 0, 1, 1);


		mainGridLayout->addWidget(mainWidget, 0, 0, 1, 1);

		QWidget::setTabOrder(EntityName, EntityType);
		QWidget::setTabOrder(EntityType, OK);
		QWidget::setTabOrder(OK, Cancel);
		QWidget::setTabOrder(Cancel, Parents);
		QWidget::setTabOrder(Parents, NavigateBack);
		QWidget::setTabOrder(NavigateBack, NavigateForward);
		QWidget::setTabOrder(NavigateForward, NavigateUp);
		QWidget::setTabOrder(NavigateUp, CreateFolder);
		QWidget::setTabOrder(CreateFolder, Favorites);
		QWidget::setTabOrder(Favorites, Files);

		retranslateUi(pqFileDialog);

		OK->setDefault(true);


		QMetaObject::connectSlotsByName(pqFileDialog);
	} // setupUi

	void retranslateUi(QWidget* pqFileDialog)
	{
		pqFileDialog->setWindowTitle(QCoreApplication::translate("pqFileDialog", "Dialog", nullptr));
		label_3->setText(QCoreApplication::translate("pqFileDialog", "Look in:", nullptr));
		NavigateBack->setText(QString());
		NavigateForward->setText(QString());
		NavigateUp->setText(QString());
		CreateFolder->setText(QString());
#if QT_CONFIG(tooltip)
		ShowDetail->setToolTip(QCoreApplication::translate("pqFileDialog", "Toggle file details. This may affect performance for large directories on certain file systems.", nullptr));
#endif // QT_CONFIG(tooltip)
		ShowDetail->setText(QString());
#if QT_CONFIG(tooltip)
		GroupFiles->setToolTip(QCoreApplication::translate("pqFileDialog", "Toggle file sequence grouping.", nullptr));
#endif // QT_CONFIG(tooltip)
		GroupFiles->setText(QString());
		favoritesLabel->setText(QCoreApplication::translate("pqFileDialog", "Favorites", nullptr));
#if QT_CONFIG(tooltip)
		AddCurrentDirectoryToFavorites->setToolTip(QCoreApplication::translate("pqFileDialog", "Add current directory to favorites", nullptr));
#endif // QT_CONFIG(tooltip)
		AddCurrentDirectoryToFavorites->setText(QString());
#if QT_CONFIG(tooltip)
		ResetFavortiesToSystemDefault->setToolTip(QCoreApplication::translate("pqFileDialog", "Clear favorites", nullptr));
#endif // QT_CONFIG(tooltip)
		ResetFavortiesToSystemDefault->setText(QString());
#if QT_CONFIG(tooltip)
		favoritesSearchBar->setToolTip(QCoreApplication::translate("pqFileDialog", "Filter favorites panel", nullptr));
#endif // QT_CONFIG(tooltip)
		favoritesSearchBar->setPlaceholderText(QCoreApplication::translate("pqFileDialog", "Filter favorites", nullptr));
		locationsLabel->setText(QCoreApplication::translate("pqFileDialog", "Locations", nullptr));
		recentLabel->setText(QCoreApplication::translate("pqFileDialog", "Recent", nullptr));
		EntityNameLabel->setText(QCoreApplication::translate("pqFileDialog", "File name:", nullptr));
		EntityTypeLabel->setText(QCoreApplication::translate("pqFileDialog", "Files of type:", nullptr));
		OK->setText(QCoreApplication::translate("pqFileDialog", "OK", nullptr));
		Cancel->setText(QCoreApplication::translate("pqFileDialog", "Cancel", nullptr));
#if QT_CONFIG(tooltip)
		Navigate->setToolTip(QCoreApplication::translate("pqFileDialog", "Go to the selected directory, instead of opening it", nullptr));
#endif // QT_CONFIG(tooltip)
		Navigate->setText(QCoreApplication::translate("pqFileDialog", "Navigate", nullptr));
	} // retranslateUi

};

namespace Ui {
	class pqFileDialog : public Ui_pqFileDialog {};
} // namespace Ui

QT_END_NAMESPACE

