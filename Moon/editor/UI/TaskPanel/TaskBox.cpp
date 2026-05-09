#include "editor/UI/TaskPanel/TaskBox.h"
#include <QFrame>
namespace MOON {
    TaskBox::TaskBox(const QString& title, QWidget* parent)
        : QWidget(parent)
    {
        setContentsMargins(0, 0, 0, 6);
        m_layout = new QVBoxLayout(this);
        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(0);

        // 仿 FreeCAD 蓝色标题栏
        m_titleBtn = new QPushButton(title);
        m_titleBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #4a86b8;
            color: white;
            text-align: left;
            padding: 4px 8px;
            border: none;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #5a96c8;
        }
    )");

        m_content = new QWidget;
        QVBoxLayout* contLay = new QVBoxLayout(m_content);
        contLay->setContentsMargins(8, 6, 8, 6);

        m_layout->addWidget(m_titleBtn);
        m_layout->addWidget(m_content);

        connect(m_titleBtn, &QPushButton::clicked, this, &TaskBox::toggleExpand);
    }

    void TaskBox::setContent(QWidget* w)
    {
        if (!m_content->layout()) return;
        m_content->layout()->addWidget(w);
    }

    void TaskBox::toggleExpand()
    {
        m_expanded = !m_expanded;
        m_content->setVisible(m_expanded);
    }
}