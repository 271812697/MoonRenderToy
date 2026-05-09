#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
namespace MOON {
    class TaskBox : public QWidget
    {
        Q_OBJECT
    public:
        explicit TaskBox(const QString& title, QWidget* parent = nullptr);
        void setContent(QWidget* w);

    private slots:
        void toggleExpand();

    private:
        QPushButton* m_titleBtn;
        QWidget* m_content;
        QVBoxLayout* m_layout;
        bool         m_expanded = true;
    };
}
