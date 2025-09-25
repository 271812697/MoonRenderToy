#pragma once
#include <QStyledItemDelegate>
#include <QIcon>

namespace MOON
{
    class EntityTreeViewStyleDelegate : public QStyledItemDelegate
    {
        Q_OBJECT
    public:
        explicit EntityTreeViewStyleDelegate(QObject* parent = nullptr);

        // 设置自定义图标
        void setCheckIcons(const QIcon& checkedIcon, const QIcon& uncheckedIcon,
            const QIcon& tristateIcon = QIcon());
      
        QIcon getIconForState(Qt::CheckState state) const;
    protected:
        void paint(QPainter* painter, const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;

        QSize sizeHint(const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;

    private:
        QIcon m_checkedIcon;      // 选中状态图标
        QIcon m_uncheckedIcon;    // 未选中状态图标
        QIcon m_tristateIcon;     // 三态状态图标（可选）
    };


}
