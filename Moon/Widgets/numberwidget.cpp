#include "Widgets/numberwidget.h"
#include "Widgets/utils.h"

#include <QEvent>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QPolygon>
#include <QSignalBlocker>
#include <QStyle>
#include <QStyleOptionFrame>

#include <algorithm>
#include <cmath>

namespace MOON {

namespace {

// Compact non-scientific representation, e.g. 0.123456789 -> "0.12345679".
QString formatAsNonscientific(double value) {
    const int visibleDigits = 8;
    const double scientificRepThreshold = 1e-6;
    if (std::abs(value) < scientificRepThreshold) {
        return QString::number(value, 'g', visibleDigits + 1);
    }
    const int intDigits =
        static_cast<int>(std::floor(std::log10(std::max(std::abs(value), 1.0)))) + 1;
    QString str = QString::number(value, 'f', std::max(visibleDigits + 1 - intDigits, 1));
    if (str.contains('.')) {
        while (str.endsWith('0')) {
            str.chop(1);
        }
        if (str.endsWith('.')) {
            str.chop(1);
        }
    }
    return str;
}

}  // namespace

NumberWidget::NumberWidget(QWidget* parent)
    : QLineEdit(parent), minimumWidth_{emToPx(this, 4)} {
    setObjectName("NumberWidget");
    updateState(FocusAction::ClearFocus);
    setFocusPolicy(Qt::TabFocus);
    setMouseTracking(true);
    QSizePolicy sp = sizePolicy();
    sp.setHorizontalPolicy(QSizePolicy::Minimum);
    setSizePolicy(sp);

    connect(this, &QLineEdit::textEdited, this, [this](const QString& text) {
        setProperty("invalid", valueFromTextValid(text) ? QVariant{} : QVariant{true});
        style()->unpolish(this);
        style()->polish(this);
    });
}

QSize NumberWidget::sizeHint() const {
    if (cachedMinimumSizeHint_.isEmpty()) {
        cachedMinimumSizeHint_ = calcMinimumSize();
    }
    return cachedMinimumSizeHint_;
}

QSize NumberWidget::minimumSizeHint() const {
    if (cachedMinimumSizeHint_.isEmpty()) {
        cachedMinimumSizeHint_ = calcMinimumSize();
    }
    return cachedMinimumSizeHint_;
}

QSize NumberWidget::calcMinimumSize() const {
    ensurePolished();
    const QSize hint(minimumWidth_, QLineEdit::minimumSizeHint().height());
    QStyleOptionFrame opt;
    initStyleOption(&opt);
    return style()->sizeFromContents(QStyle::CT_LineEdit, &opt, hint, this);
}

void NumberWidget::setPrefix(const QString& prefix) {
    prefix_ = prefix;
    updateText();
}

const QString& NumberWidget::getPrefix() const {
    return prefix_;
}

void NumberWidget::setPostfix(const QString& postfix) {
    postfix_ = postfix;
    updateText();
}

void NumberWidget::setRange(double min, double max) {
    minValue_ = min;
    maxValue_ = max;
}

void NumberWidget::setIncrement(double inc) {
    increment_ = inc;
}

void NumberWidget::setWrapping(bool wrapping) {
    wrapping_ = wrapping;
}

void NumberWidget::setInteractionMode(Interaction mode) {
    mode_ = mode;
}

void NumberWidget::setPercentageBarVisible(bool visible) {
    percentageBarVisible_ = visible;
    update();
}

double NumberWidget::getValue() const {
    return value_;
}

void NumberWidget::setValue(double value) {
    if (value != value_) {
        value_ = value;
        updateText();
        emit valueChanged();
    }
}

void NumberWidget::initValue(double value) {
    value_ = value;
    updateText();
}

bool NumberWidget::event(QEvent* event) {
    if (event->type() == QEvent::ReadOnlyChange) {
        updateHoverState(mapFromGlobal(QCursor::pos()));
        const QSignalBlocker blocker{this};
        updateText();
    } else if (!isReadOnly() && isEnabled()) {
        if (event->type() == QEvent::FocusIn) {
            updateState(FocusAction::SetFocus);
        } else if (event->type() == QEvent::FocusOut) {
            if (updateValueFromText(text())) {
                emit valueChanged();
            }
            updateState(FocusAction::ClearFocus);
        } else if (event->type() == QEvent::KeyPress &&
                   handleKeyEvent(static_cast<QKeyEvent*>(event))) {
            return true;
        }
    }
    return QLineEdit::event(event);
}

bool NumberWidget::handleKeyEvent(QKeyEvent* keyEvent) {
    if (keyEvent->key() == Qt::Key_Escape) {
        // cancel editing
        const QSignalBlocker blocker{this};
        updateText();
        clearFocus();
        return true;
    } else if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
        // commit changes
        clearFocus();
        return true;
    } else if (keyEvent->key() == Qt::Key_Up) {
        if (incrementValue()) {
            updateText();
            emit valueChanged();
            update();
        }
        return true;
    } else if (keyEvent->key() == Qt::Key_Down) {
        if (decrementValue()) {
            updateText();
            emit valueChanged();
            update();
        }
        return true;
    }
    return false;
}

void NumberWidget::mousePressEvent(QMouseEvent* event) {
    if (!hasFocus() && !isReadOnly() && isEnabled() && event->button() == Qt::LeftButton) {
        state_.previousPos = event->pos();
        state_.dragging = false;
        event->accept();
    } else {
        QLineEdit::mousePressEvent(event);
    }
}

void NumberWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (!hasFocus() && !isReadOnly() && isEnabled() && event->button() == Qt::LeftButton) {
        event->accept();
        if (!state_.dragging) {
            switch (state_.hover) {
                case HoverZone::PositiveInc:
                    incrementValue();
                    emit valueChanged();
                    update();
                    break;
                case HoverZone::NegativeInc:
                    decrementValue();
                    emit valueChanged();
                    update();
                    break;
                case HoverZone::Center:
                default:
                    // mouse left click on center and no movement, start editing
                    setFocus(Qt::TabFocusReason);
                    break;
            }
        }
        state_.dragging = false;
    } else {
        QLineEdit::mouseReleaseEvent(event);
    }
}

void NumberWidget::mouseMoveEvent(QMouseEvent* event) {
    updateHoverState(event->pos());

    // ignore dragging as long as movement delta is too small
    if (!hasFocus() && !isReadOnly() && isEnabled() && event->buttons().testFlag(Qt::LeftButton)) {
        event->accept();
        const int delta = (event->pos() - state_.previousPos).x();
        if (!state_.dragging) {
            const int minimalMovement = 3;
            state_.dragging = std::abs(delta) > minimalMovement;
            if (state_.dragging && mode_ == Interaction::Dragging) {
                setCursor(Qt::SizeHorCursor);
                state_.hover = HoverZone::Center;
                state_.modifiers = QGuiApplication::queryKeyboardModifiers();
                initDragValue();
            }
        } else if (mode_ == Interaction::Dragging) {
            double deltaStep = static_cast<double>(delta);
            const auto modifiers = QGuiApplication::queryKeyboardModifiers();
            if (modifiers != state_.modifiers) {
                initDragValue();
                state_.previousPos = event->pos();
                state_.modifiers = modifiers;
            }
            if (modifiers.testFlag(increasedStepModifier)) {
                deltaStep *= increasedStepSize;
            } else if (modifiers.testFlag(decreasedStepModifier)) {
                deltaStep *= decreasedStepSize;
            }
            applyDragDelta(deltaStep);
            emit valueChanged();
            update();
        }
    } else {
        QLineEdit::mouseMoveEvent(event);
    }
}

void NumberWidget::paintEvent(QPaintEvent* event) {
    const int borderWidth = std::max(1, emToPPx(fontMetrics(), 0.0625));
    const int arrowWidth = height() * 2 / 3;
    QStyleOptionFrame panel;
    initStyleOption(&panel);
    const bool hover = panel.state.testFlag(QStyle::State_MouseOver);

    if (!hasFocus() && !isReadOnly() && isEnabled()) {
        // Paint the idle state directly instead of relying on the stylesheet,
        // so the first frame is identical to every later frame. The fill is
        // opaque (panel background when idle) so any stale pixels from an
        // earlier paint are always overwritten.
        const bool invalid = property("invalid").toBool();
        const QColor fill =
            hover ? QColor("#47474b") : palette().color(QPalette::Window);
        const QColor borderColor =
            invalid ? QColor("#801717") : (hover ? QColor("#268bd2") : QColor());

        QPainter painter{this};
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

        painter.setPen(Qt::NoPen);
        painter.setBrush(fill);
        painter.drawRoundedRect(rect(), 3, 3);

        if (borderColor.isValid()) {
            QPen pen(borderColor, borderWidth);
            painter.setPen(pen);
            painter.setBrush(Qt::NoBrush);
            painter.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 3, 3);
        }

        const QRect r = style()->subElementRect(QStyle::SE_LineEditContents, &panel, this);
        painter.setPen(Qt::NoPen);
        painter.setClipRect(r);

        const QRect interiorRect = rect().adjusted(borderWidth, borderWidth, -borderWidth, -borderWidth);

        if (percentageBarVisible_) {
            drawPercentageBar(painter, interiorRect, hover);
        }
        if (hover && mode_ == Interaction::Dragging) {
            drawArrows(painter, interiorRect, arrowWidth);
        }

        painter.setFont(font());
        painter.setPen(palette().color(QPalette::Text));
        painter.drawText(event->rect(), Qt::AlignCenter, getPrefixedText());
    } else {
        QLineEdit::paintEvent(event);
    }
}

void NumberWidget::drawPercentageBar(QPainter& painter, const QRect& rect, bool hover) const {
    const auto [percentage, state] = getPercentageBar();
    if (percentage.has_value() && state != PercentageBar::Invalid) {
        QRect centerRect;
        if (state == PercentageBar::Regular) {
            const int barWidth = static_cast<int>(rect.width() * *percentage);
            centerRect = QRect{rect.topLeft(), QSize{barWidth, rect.height()}};
        } else if (state == PercentageBar::Symmetric) {
            const int barWidth = static_cast<int>(rect.width() * *percentage * 0.5);
            const int minBarWidth = emToPPx(fontMetrics(), 0.0625);
            centerRect = QRect(QPoint(std::min(barWidth, -minBarWidth), rect.top()),
                               QPoint(std::max(barWidth, minBarWidth), rect.bottom()));
            centerRect.translate(rect.width() / 2, 0);
        }
        // use regular Inviwo blue on hover, slightly darkened otherwise
        painter.setBrush(QBrush{hover ? QColor{"#1e70a8"} : QColor{"#103a57"}});
        painter.drawRect(centerRect);
    }
}

void NumberWidget::drawArrows(QPainter& painter, const QRect& interior, int arrowWidth) const {
    const QBrush bgActive{"#185987"};
    const QColor arrowColor{"#999999"};
    QRect arrowRect{interior.topLeft(), QSize{arrowWidth, interior.height()}};

    // Thin stroked chevron matching Inviwo's arrow icons: roughly 38% of the
    // zone width and 77% of its height, so it stays compact in the corner.
    const auto drawChevron = [&](const QRect& r, bool left) {
        const int padX = r.width() * 31 / 100;
        const int padY = r.height() * 11 / 100;
        QPolygon polygon;
        if (left) {
            polygon << QPoint(r.right() - padX, r.top() + padY) << QPoint(r.left() + padX, r.center().y())
                    << QPoint(r.right() - padX, r.bottom() - padY);
        } else {
            polygon << QPoint(r.left() + padX, r.top() + padY) << QPoint(r.right() - padX, r.center().y())
                    << QPoint(r.left() + padX, r.bottom() - padY);
        }
        QPen pen(arrowColor, std::max(1, r.width() / 9));
        pen.setCapStyle(Qt::FlatCap);
        pen.setJoinStyle(Qt::MiterJoin);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawPolyline(polygon);
    };

    if (state_.hover == HoverZone::NegativeInc) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(bgActive);
        painter.drawRect(arrowRect);
    }
    drawChevron(arrowRect, true);

    arrowRect.moveRight(interior.right());
    if (state_.hover == HoverZone::PositiveInc) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(bgActive);
        painter.drawRect(arrowRect);
    }
    drawChevron(arrowRect, false);
}

void NumberWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::EnabledChange) {
        updateText();
        update();
    } else if (event->type() == QEvent::StyleChange) {
        cachedMinimumSizeHint_ = QSize();
        updateGeometry();
    }
    QLineEdit::changeEvent(event);
}

void NumberWidget::updateText() {
    const QString str = (isReadOnly() || !isEnabled()) ? getPrefixedText() : getTextFromValue(true);
    if (str != text()) {
        setText(str);
    }
}

QString NumberWidget::getPrefixedText() const {
    return QString("%1%2%3%4")
        .arg(prefix_)
        .arg(prefix_.isEmpty() ? "" : ": ")
        .arg(getTextFromValue(false))
        .arg(postfix_);
}

void NumberWidget::updateState(FocusAction action) {
    if (action == FocusAction::SetFocus) {
        // update textual representation
        updateText();
    }
    setProperty("invalid", QVariant{});
    style()->unpolish(this);
    style()->polish(this);
    setAlignment(action == FocusAction::SetFocus ? Qt::AlignLeft : Qt::AlignCenter);

    updateHoverState(mapFromGlobal(QCursor::pos()));
}

auto NumberWidget::getHoverZone(QPoint mousePos) const -> HoverZone {
    const int arrowWidth = height() * 2 / 3;

    if (hasFocus() || isReadOnly() || !isEnabled() || mode_ == Interaction::NoDragging) {
        return HoverZone::Invalid;
    }
    if (mousePos.x() < arrowWidth) {
        return HoverZone::NegativeInc;
    } else if (mousePos.x() > width() - arrowWidth) {
        return HoverZone::PositiveInc;
    }
    return HoverZone::Center;
}

void NumberWidget::updateHoverState(QPoint mousePos) {
    if (state_.dragging) {
        return;
    }

    if (auto hoverZone = getHoverZone(mousePos); hoverZone != state_.hover) {
        state_.hover = hoverZone;
        update();

        switch (state_.hover) {
            case HoverZone::Invalid:
                setCursor(Qt::IBeamCursor);
                break;
            case HoverZone::Center:
                setCursor(Qt::SizeHorCursor);
                break;
            case HoverZone::NegativeInc:
            case HoverZone::PositiveInc:
            default:
                unsetCursor();
                break;
        }
    }
}

bool NumberWidget::incrementValue() {
    double uiIncrement = getUIIncrement();
    const auto modifiers = QGuiApplication::queryKeyboardModifiers();
    if (modifiers.testFlag(increasedStepModifier)) {
        uiIncrement *= increasedStepSize;
    } else if (modifiers.testFlag(decreasedStepModifier)) {
        uiIncrement *= decreasedStepSize;
    }
    if (maxValue_ - uiIncrement < value_) {
        if (wrapping_) {
            const double delta = maxValue_ - value_;
            return updateValue(minValue_ + uiIncrement - delta);
        }
        return updateValue(maxValue_);
    }
    return updateValue(value_ + uiIncrement);
}

bool NumberWidget::decrementValue() {
    double uiIncrement = getUIIncrement();
    const auto modifiers = QGuiApplication::queryKeyboardModifiers();
    if (modifiers.testFlag(increasedStepModifier)) {
        uiIncrement *= increasedStepSize;
    } else if (modifiers.testFlag(decreasedStepModifier)) {
        uiIncrement *= decreasedStepSize;
    }
    if (minValue_ + uiIncrement > value_) {
        if (wrapping_) {
            return updateValue(maxValue_ - uiIncrement + value_ - minValue_);
        }
        return updateValue(minValue_);
    }
    return updateValue(value_ - uiIncrement);
}

bool NumberWidget::applyDragDelta(double deltaSteps) {
    if (deltaSteps == 0.0) {
        return false;
    }

    const bool positiveDelta = !std::signbit(deltaSteps);
    double absDelta = std::abs(deltaSteps * getUIIncrement());
    if (wrapping_) {
        absDelta = std::fmod(absDelta, maxValue_ - minValue_);
        if (positiveDelta && maxValue_ - absDelta < initialDragValue_) {
            absDelta -= maxValue_ - minValue_;
        } else if (!positiveDelta && minValue_ + absDelta > initialDragValue_) {
            absDelta = maxValue_ - minValue_ - absDelta;
        }
        if (positiveDelta) {
            return updateValue(initialDragValue_ + absDelta);
        }
        return updateValue(initialDragValue_ - absDelta);
    }

    if (positiveDelta) {
        return updateValue(std::min(initialDragValue_ + absDelta, maxValue_));
    }
    return updateValue(std::max(initialDragValue_ - absDelta, minValue_));
}

void NumberWidget::initDragValue() {
    // Keep track of the value at drag start so the total delta is absolute.
    initialDragValue_ = value_;
}

bool NumberWidget::valueFromTextValid(const QString& str) const {
    bool ok = false;
    const double newValue = str.toDouble(&ok);
    return ok && newValue == std::clamp(newValue, minValue_, maxValue_);
}

bool NumberWidget::updateValueFromText(const QString& str) {
    bool ok = false;
    const double newValue = str.toDouble(&ok);
    if (!ok) {
        return false;
    }
    const double clamped = std::clamp(newValue, minValue_, maxValue_);
    const bool updated = (clamped != value_);
    value_ = clamped;
    return updated;
}

QString NumberWidget::getTextFromValue(bool precise) const {
    if (precise) {
        return formatAsNonscientific(value_);
    }
    return QString::number(value_, 'f', MOON::decimals(increment_));
}

double NumberWidget::getUIIncrement() const {
    if (mode_ == Interaction::NoDragging) {
        return increment_;
    }
    return (maxValue_ - minValue_) / static_cast<double>(std::max(width(), 1));
}

std::pair<std::optional<double>, NumberWidget::PercentageBar> NumberWidget::getPercentageBar() const {
    const auto state = [&]() {
        if (minValue_ == -maxValue_) {
            return PercentageBar::Symmetric;
        }
        return PercentageBar::Regular;
    }();

    std::optional<double> percentage;
    if (state == PercentageBar::Symmetric && maxValue_ != 0.0) {
        percentage = value_ / maxValue_;
    } else if (state == PercentageBar::Regular && maxValue_ != minValue_) {
        percentage = (value_ - minValue_) / (maxValue_ - minValue_);
    }
    return {percentage, state};
}

bool NumberWidget::updateValue(double value) {
    const double clamped = std::clamp(value, minValue_, maxValue_);
    if (value_ != clamped) {
        value_ = clamped;
        return true;
    }
    return false;
}

}  // namespace MOON
