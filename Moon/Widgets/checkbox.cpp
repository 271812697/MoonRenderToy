#include "checkbox.h"
#include <QPainter>
#include <QMouseEvent>
#include <QStyleOptionButton>
#include <QFontMetrics>
#include <iostream>

SlidingCheckBox::SlidingCheckBox(QWidget* parent, const QString& text)
    : QAbstractButton(parent),
    m_circleX(5),
    m_circleRadius(6),
    m_rectHeight(15),
    m_rectWidth(60),
    m_checkedColor(QColor(46, 204, 113)),    // 绿色
    m_uncheckedColor(QColor(200, 200, 200)), // 灰色
    m_circleColor(Qt::white)                 // 白色滑块
{
    setText(text);
    setCheckable(true);
    m_bgColor = m_uncheckedColor;

    // 创建动画对象
    m_anim = new QPropertyAnimation(this, "circleX", this);
    m_anim->setDuration(300); // 动画持续时间
    m_anim->setEasingCurve(QEasingCurve::InOutCubic); // 缓动曲线
    int startX = checked ? (m_circleRadius + 1) : (m_rectWidth - m_circleRadius - 1);
    int endX = checked ? (m_rectWidth - m_circleRadius - 1) : (m_circleRadius + 1);
    m_anim->setStartValue(startX);
    m_anim->setEndValue(endX);
    m_anim->start();   

    // 背景颜色动画
    colorAnim = new QPropertyAnimation(this, "bgColor", this);
    colorAnim->setDuration(500);
    colorAnim->setEasingCurve(QEasingCurve::InOutCubic);
    colorAnim->setStartValue(checked ? m_uncheckedColor : m_checkedColor);
    colorAnim->setEndValue(checked ? m_checkedColor : m_uncheckedColor);
    colorAnim->start();
}

void SlidingCheckBox::paintEvent(QPaintEvent* event)
{
   
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true); // 抗锯齿

    // 绘制背景圆角矩形
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_bgColor);
    QRect rect(0, (height() - m_rectHeight) / 2, m_rectWidth, m_rectHeight);
    painter.drawRoundedRect(rect, m_rectHeight / 2, m_rectHeight / 2);

    
    // 绘制圆形滑块
    painter.setBrush(m_circleColor);
    painter.drawEllipse(QPoint(m_circleX, height() / 2), m_circleRadius, m_circleRadius);

    // 绘制文本
    if (!text().isEmpty()) {
        painter.setPen(palette().text().color());
        int textX = 60; // 文本起始X坐标
        int textY = height() / 2 + fontMetrics().ascent() / 2-4;
        painter.drawText(textX, textY, text());
    }
}

void SlidingCheckBox::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && rect().contains(event->pos())) {
        // 切换选中状态
        checked = !checked;
        onStateChanged();
    }
    QAbstractButton::mouseReleaseEvent(event);
}

void SlidingCheckBox::resizeEvent(QResizeEvent* event)
{
    QAbstractButton::resizeEvent(event);
    
    // 确保滑块位置正确
    if (isChecked()) {
        m_circleX = m_rectWidth - m_circleRadius - 1;
    }
    else {
        m_circleX = m_circleRadius + 1;
    }
}

void SlidingCheckBox::onStateChanged()
{
    // 计算动画的起始和结束位置
    int startX = checked ?  (m_circleRadius + 1) :(m_rectWidth - m_circleRadius - 1);
    int endX = checked ? (m_rectWidth - m_circleRadius - 1) : (m_circleRadius + 1);

    // 启动位置动画
    m_anim->setStartValue(startX);
    m_anim->setEndValue(endX);
    m_anim->start();

    // 启动颜色动画


    colorAnim->setStartValue(checked ? m_uncheckedColor : m_checkedColor);
    colorAnim->setEndValue(checked ? m_checkedColor : m_uncheckedColor);
    colorAnim->start();
}