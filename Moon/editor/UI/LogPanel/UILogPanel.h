#ifndef UI_LOGPANEL_H
#define UI_LOGPANEL_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LogPanel
{
public:
    QAction *m_actionCopyContent;
    QWidget *dockWidgetContents;
    QHBoxLayout *horizontalLayout;
    QListWidget *m_logList;
    QVBoxLayout *verticalLayout_3;
    QCheckBox *checkBox_5;
    QCheckBox *checkBox;
    QPushButton *m_clear;
    QSpacerItem *verticalSpacer_3;

    void setupUi(QDockWidget *LogPanel)
    {
        if (LogPanel->objectName().isEmpty())
            LogPanel->setObjectName(QStringLiteral("LogPanel"));
        LogPanel->resize(852, 250);
        m_actionCopyContent = new QAction(LogPanel);
        m_actionCopyContent->setObjectName(QStringLiteral("m_actionCopyContent"));
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QStringLiteral("dockWidgetContents"));
        horizontalLayout = new QHBoxLayout(dockWidgetContents);
        horizontalLayout->setSpacing(3);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(3, 3, 3, 3);
        m_logList = new QListWidget(dockWidgetContents);
        m_logList->setObjectName(QStringLiteral("m_logList"));
        m_logList->setContextMenuPolicy(Qt::CustomContextMenu);
        m_logList->setStyleSheet("QListWidget::item { "
            "height: 40px;"
            "vertical-align: middle; "
            "text-align: left;"
            "}");
        horizontalLayout->addWidget(m_logList);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setSpacing(3);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        checkBox_5 = new QCheckBox(dockWidgetContents);
        checkBox_5->setObjectName(QStringLiteral("checkBox_5"));
        checkBox_5->setChecked(true);

        verticalLayout_3->addWidget(checkBox_5);

        checkBox = new QCheckBox(dockWidgetContents);
        checkBox->setObjectName(QStringLiteral("checkBox"));
        checkBox->setMinimumSize(QSize(120, 0));
        checkBox->setChecked(true);

        verticalLayout_3->addWidget(checkBox);

        m_clear = new QPushButton(dockWidgetContents);
        m_clear->setObjectName(QStringLiteral("m_clear"));

        verticalLayout_3->addWidget(m_clear);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_3->addItem(verticalSpacer_3);


        horizontalLayout->addLayout(verticalLayout_3);

        LogPanel->setWidget(dockWidgetContents);

        retranslateUi(LogPanel);

        QMetaObject::connectSlotsByName(LogPanel);
    } // setupUi

    void retranslateUi(QDockWidget *LogPanel)
    {
        LogPanel->setWindowTitle(QApplication::translate("LogPanel", "Log", nullptr));
        m_actionCopyContent->setText(QApplication::translate("LogPanel", "Copy", nullptr));
        checkBox_5->setText(QApplication::translate("LogPanel", "Error", nullptr));
        checkBox->setText(QApplication::translate("LogPanel", "Warning", nullptr));
        m_clear->setText(QApplication::translate("LogPanel", "Clean", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LogPanel: public Ui_LogPanel {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGPANEL_H
