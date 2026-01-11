#ifndef SLIDINGCHECKBOX_H
#define SLIDINGCHECKBOX_H

#include <QAbstractButton>
#include <QPropertyAnimation>
#include <QColor>

class SlidingCheckBox : public QAbstractButton
{
    Q_OBJECT
        Q_PROPERTY(int circleX READ circleX WRITE setCircleX)
        Q_PROPERTY(QColor bgColor READ bgColor WRITE setBgColor)

public:
    explicit SlidingCheckBox(QWidget* parent = nullptr, const QString& text = "");


    QSize sizeHint() const override { return QSize(60 + fontMetrics().width(text()) + 10, 30); }
    QSize minimumSizeHint() const override { return QSize(60 + fontMetrics().width(text()) + 10, 30); }

    int circleX() const { return m_circleX; }
    void setCircleX(int x) { m_circleX = x; update(); }

    QColor bgColor() const { return m_bgColor; }
    void setBgColor(const QColor& color) { m_bgColor = color; update(); }

  
protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private :
    void onStateChanged();
private:

    int m_circleX;              // 圆形滑块X坐标
    int m_circleRadius;         // 圆形滑块半径
    int m_rectHeight;           // 背景矩形高度
    int m_rectWidth;
    QColor m_bgColor;           // 背景颜色
    QColor m_checkedColor;      // 选中状态颜色
    QColor m_uncheckedColor;    // 未选中状态颜色
    QColor m_circleColor;       // 圆形滑块颜色
    QPropertyAnimation* m_anim; // 动画对象
    QPropertyAnimation* colorAnim; // 动画对象
    bool checked = false;
};

#endif // SLIDINGCHECKBOX_H