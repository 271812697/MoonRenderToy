#include "QtWidgets/CollapsibleWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QMouseEvent>
namespace MOON {

	class CollapsibleWidget::CollapsibleWidgetInternal {
	public:
		// 设置折叠/展开状态
		void setExpanded(bool expanded)
		{
			m_isExpanded = expanded;
			m_contentWidget->setVisible(expanded);
			m_iconLabel->setText(expanded ? "a" : "b");
		}

		// 切换折叠/展开状态
		void toggleExpand()
		{
			setExpanded(!m_isExpanded);
		}
		CollapsibleWidgetInternal(CollapsibleWidget* widget,QString title):mSelf(widget) {
			// 根布局：垂直布局（标题栏 + 内容区域）
			QVBoxLayout* rootLayout = new QVBoxLayout(mSelf);
			rootLayout->setContentsMargins(0, 0, 0, 0);
			rootLayout->setSpacing(0);

			// ---------------------- 标题栏 ----------------------
			m_titleWidget = new QWidget(mSelf);
			m_titleWidget->setStyleSheet("background-color: #e0e0e0;");
			QHBoxLayout* titleLayout = new QHBoxLayout(m_titleWidget);
			titleLayout->setContentsMargins(10, 5, 10, 5);

			// 折叠状态图标（▶/▼）
			m_iconLabel = new QLabel(mSelf);
			m_iconLabel->setText("▼");
			titleLayout->addWidget(m_iconLabel);

			// 标题文本
			QLabel* titleLabel = new QLabel(title, mSelf);
			titleLayout->addWidget(titleLabel);
			titleLayout->addStretch(); // 推到右侧

			// 标题栏设置鼠标样式（手型），提示可点击
			m_titleWidget->setCursor(Qt::PointingHandCursor);

			// ---------------------- 内容区域 ----------------------
			m_contentWidget = new QWidget(mSelf);
			m_contentLayout = new QVBoxLayout(m_contentWidget);
			m_contentLayout->setContentsMargins(10, 10, 10, 10);
			m_contentLayout->setSpacing(8);

			// 添加到根布局
			rootLayout->addWidget(m_titleWidget);
			rootLayout->addWidget(m_contentWidget);

			// 初始展开
			setExpanded(true);
		}
		~CollapsibleWidgetInternal() {

		}
	private:
		friend class CollapsibleWidget;
		CollapsibleWidget* mSelf = nullptr;
		bool m_isExpanded;                  // 是否展开
		QWidget* m_titleWidget;             // 标题栏控件
		QLabel* m_iconLabel;                // 折叠图标
		QWidget* m_contentWidget;           // 内容区域
		QVBoxLayout* m_contentLayout;       // 内容布局

	};
	CollapsibleWidget::CollapsibleWidget(const QString& title, QWidget* parent) :
		QWidget(parent),mInternal(new CollapsibleWidgetInternal(this,title))
	{
		mInternal->m_isExpanded = true;
	}
	CollapsibleWidget::~CollapsibleWidget()
	{
		delete mInternal;
	}
	QVBoxLayout* CollapsibleWidget::contentLayout()
	{
		return mInternal->m_contentLayout;
	}
	void CollapsibleWidget::mousePressEvent(QMouseEvent* event)
	{
		// 判断点击位置是否在标题栏区域内
		if (mInternal->m_titleWidget->geometry().contains(event->pos())) {
			mInternal->toggleExpand(); // 点击标题栏则切换折叠状态
		}
		QWidget::mousePressEvent(event); // 传递事件给父类
	}
}