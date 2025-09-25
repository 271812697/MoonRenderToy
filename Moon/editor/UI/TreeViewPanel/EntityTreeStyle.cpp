#include "EntityTreeStyle.h"
#include <QPainter>
namespace MOON {
	EntityTreeViewStyleDelegate::EntityTreeViewStyleDelegate(QObject* parent) : QStyledItemDelegate(parent)
	{
		
		m_checkedIcon = QIcon(":/entityTree/icons/pqEyeball.svg");
		m_uncheckedIcon = QIcon(":/entityTree/icons/pqEyeballClosed.svg");
		m_tristateIcon = QIcon(":/entityTree/icons/pqEyeballClosed.svg");
	}
	void EntityTreeViewStyleDelegate::setCheckIcons(const QIcon& checkedIcon, const QIcon& uncheckedIcon, const QIcon& tristateIcon)
	{
		m_checkedIcon = checkedIcon;
		m_uncheckedIcon = uncheckedIcon;
		m_tristateIcon = tristateIcon;
	}
    QIcon EntityTreeViewStyleDelegate::getIconForState(Qt::CheckState state) const
    {
        switch (state) {
        case Qt::Checked:
            return m_checkedIcon;
        case Qt::Unchecked:
            return m_uncheckedIcon;
        case Qt::PartiallyChecked:
            return m_tristateIcon.isNull() ? m_uncheckedIcon : m_tristateIcon;
        default:
            return QIcon();
        }
    }
	void EntityTreeViewStyleDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
        // 只处理有复选框的列
        if (index.flags() & Qt::ItemIsUserCheckable) {
            QVariant value = index.data(Qt::CheckStateRole);
            if (value.isValid()) {
                Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());

                // 绘制背景和其他元素
                QStyleOptionViewItem opt = option;
                initStyleOption(&opt, index);
              
                // 绘制项目背景
                opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

                // 计算复选框位置
                QRect checkRect = opt.widget->style()->subElementRect(
                    QStyle::SE_ItemViewItemCheckIndicator, &opt, opt.widget);

                // 根据状态选择图标
                QIcon icon;
                switch (state) {
                case Qt::Checked:
                    icon = m_checkedIcon;
                    break;
                case Qt::Unchecked:
                    icon = m_uncheckedIcon;
                    break;
                case Qt::PartiallyChecked:
                    icon = m_tristateIcon;
                    break;
                }

                // 绘制自定义图标
                if (!icon.isNull()) {
                   
                    icon.paint(painter, checkRect, Qt::AlignCenter, QIcon::Normal);
                }

                return; // 已处理，不需要默认绘制
            }
        }

        // 对于没有复选框的项目，使用默认绘制
        QStyledItemDelegate::paint(painter, option, index);
	}
	QSize EntityTreeViewStyleDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
        if (index.flags() & Qt::ItemIsUserCheckable) {
            QSize size = QStyledItemDelegate::sizeHint(option, index);  
            size.setWidth(size.width() + 8); 
            return size;
        }
        return QStyledItemDelegate::sizeHint(option, index);
	}
}