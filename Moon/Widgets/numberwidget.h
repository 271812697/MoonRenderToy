#pragma once
#include <QLineEdit>
#include <QSize>
#include <cstdint>
#include <optional>
#include <utility>

class QEvent;
class QKeyEvent;
class QMouseEvent;
class QPainter;
class QPaintEvent;

namespace MOON {

// Inviwo-style numeric editor. A QLineEdit that shows a prefixed value
// ("x: 1.25") centered while idle, paints a percentage bar in the background,
// and supports dragging to change the value. Click the center to edit the
// number directly; hover the left/right edge to step up/down.
class NumberWidget : public QLineEdit {
    Q_OBJECT
public:
    enum class Interaction : std::uint8_t { NoDragging, Dragging };
    enum class PercentageBar : std::uint8_t { Invalid, Regular, Symmetric };

    explicit NumberWidget(QWidget* parent = nullptr);

    void setPrefix(const QString& prefix);
    const QString& getPrefix() const;
    void setPostfix(const QString& postfix);

    void setRange(double min, double max);
    void setIncrement(double inc);
    void setWrapping(bool wrapping);
    void setInteractionMode(Interaction mode);
    void setPercentageBarVisible(bool visible);

    double getValue() const;
    // Update value and emit valueChanged() if it changed.
    void setValue(double value);
    // Update the displayed value without emitting valueChanged().
    void initValue(double value);

    virtual QSize sizeHint() const override;
    virtual QSize minimumSizeHint() const override;

signals:
    void valueChanged();

protected:
    virtual bool event(QEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void changeEvent(QEvent* event) override;

private:
    enum class HoverZone : std::uint8_t { Invalid, Center, NegativeInc, PositiveInc };
    enum class FocusAction : std::uint8_t { SetFocus, ClearFocus };

    bool handleKeyEvent(QKeyEvent* keyEvent);
    QSize calcMinimumSize() const;
    void drawPercentageBar(QPainter& painter, const QRect& rect, bool hover) const;
    void drawArrows(QPainter& painter, const QRect& interior, int arrowWidth) const;
    QString getPrefixedText() const;
    void updateState(FocusAction action);
    HoverZone getHoverZone(QPoint mousePos) const;
    void updateHoverState(QPoint mousePos);
    void updateText();

    bool incrementValue();
    bool decrementValue();
    bool applyDragDelta(double deltaSteps);
    void initDragValue();
    bool valueFromTextValid(const QString& str) const;
    bool updateValueFromText(const QString& str);
    QString getTextFromValue(bool precise) const;
    double getUIIncrement() const;
    std::pair<std::optional<double>, PercentageBar> getPercentageBar() const;
    bool updateValue(double value);

    static constexpr Qt::KeyboardModifier increasedStepModifier = Qt::ControlModifier;
    static constexpr Qt::KeyboardModifier decreasedStepModifier = Qt::ShiftModifier;
    static constexpr double increasedStepSize = 10.0;
    static constexpr double decreasedStepSize = 0.1;

    double value_{1.0};
    double minValue_{0.0};
    double maxValue_{2.0};
    double increment_{0.1};
    double initialDragValue_{0.0};

    bool wrapping_{false};
    bool percentageBarVisible_{true};
    Interaction mode_{Interaction::Dragging};
    QString prefix_;
    QString postfix_;
    int minimumWidth_{0};

    struct InteractionState {
        QPoint previousPos;
        Qt::KeyboardModifiers modifiers;
        bool dragging = false;
        HoverZone hover = HoverZone::Invalid;
    } state_;

    mutable QSize cachedMinimumSizeHint_;
};

}
