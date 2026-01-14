#include "Widgets/sliderwidget.h"
#include "Widgets/numberlineedit.h"     // for NumberLineEdit

#include <cmath>   // for fabs
#include <limits>  // for numeric_limits

#include <QDoubleSpinBox>  // for QDoubleSpinBox
#include <QEvent>          // for QEvent, QEvent::MouseButtonRelease
#include <QHBoxLayout>     // for QHBoxLayout
#include <QMouseEvent>     // for QMouseEvent
#include <QPoint>          // for QPoint
#include <QSignalBlocker>  // for QSignalBlocker
#include <QSizePolicy>     // for QSizePolicy, QSizePolicy::Fixed
#include <QSlider>         // for QSlider
#include <QStyle>          // for QStyle
#include <QVariant>        // for QVariant
#include <Qt>      
namespace MOON {
    namespace {

        class Slider : public QSlider {
            using QSlider::QSlider;

            virtual void wheelEvent(QWheelEvent* e) override {
                if (hasFocus()) QSlider::wheelEvent(e);
            }
        };

    }  // namespace

    BaseSliderWidgetQt::BaseSliderWidgetQt(QWidget* parent,bool intMode)
        : PropertyQtWidget(parent)
        , spinBox_(new NumberLineEdit(intMode))
        , slider_(new Slider())
        , spinnerValue_(0.0)
        , sliderValue_(0)
        
        {

        QHBoxLayout* hLayout = new QHBoxLayout();

        slider_->setOrientation(Qt::Horizontal);
        slider_->setPageStep(1);
        slider_->setMaximum(sliderMax_);
        slider_->installEventFilter(this);
        slider_->setFocusPolicy(Qt::ClickFocus);

        setFocusPolicy(spinBox_->focusPolicy());
        setFocusProxy(spinBox_);
        slider_->setFocusProxy(spinBox_);

        spinBox_->setKeyboardTracking(false);  // don't emit the valueChanged() signal while typing

        hLayout->addWidget(slider_);
        hLayout->addWidget(spinBox_);
        hLayout->setContentsMargins(0, 0, 0, 0);
        hLayout->setSpacing(refSpacePx(this));
        hLayout->setStretch(0, 3);
        hLayout->setStretch(1, 1);

        setLayout(hLayout);
        connect(slider_, &QSlider::valueChanged, this, &BaseSliderWidgetQt::updateFromSlider);
        connect(spinBox_, static_cast<void (QDoubleSpinBox::*)()>(&QDoubleSpinBox::editingFinished),
            this, &BaseSliderWidgetQt::updateFromSpinBox);
        connect(this, &BaseSliderWidgetQt::valueChanged, [this]() {
			OnValueChanged();
            });

        QSizePolicy sp = sizePolicy();
        sp.setVerticalPolicy(QSizePolicy::Fixed);
        setSizePolicy(sp);
    }

    void BaseSliderWidgetQt::setWrapping(bool wrap) { spinBox_->setWrapping(wrap); }

    bool BaseSliderWidgetQt::wrapping() const { return spinBox_->wrapping(); }

    void BaseSliderWidgetQt::applyInit() {
        updateSlider();
        updateSpinBox();
    }

    void BaseSliderWidgetQt::applyValue() {
        applyInit();
        emit valueChanged();
    }
    void BaseSliderWidgetQt::applyMinValue() {
        QSignalBlocker spinBlock(spinBox_);
        QSignalBlocker slideBlock(slider_);


        {
            spinBox_->setMinimum(transformMinValueToSpinner());
        }
        slider_->setMinimum(transformMinValueToSlider());
        updateSlider();
    }
    void BaseSliderWidgetQt::applyMaxValue() {
        QSignalBlocker spinBlock(spinBox_);
        QSignalBlocker slideBlock(slider_);

       {
            spinBox_->setMaximum(transformMaxValueToSpinner());
        }
        slider_->setMaximum(transformMaxValueToSlider());
        updateSlider();
    }
    void BaseSliderWidgetQt::applyIncrement() {
        QSignalBlocker spinBlock(spinBox_);
        QSignalBlocker slideBlock(slider_);

        spinBox_->setIncrement(transformIncrementToSpinner());
        spinBox_->setDecimals(transformIncrementToSpinnerDecimals());
        slider_->setSingleStep(transformIncrementToSlider());
    }

    void BaseSliderWidgetQt::updateFromSlider() {
        const int newValue = slider_->value();
        if (newValue != sliderValue_) {
            sliderValue_ = newValue;
            updateOutOfBounds();
            newSliderValue(sliderValue_);
            updateSpinBox();
            emit valueChanged();
        }
    }

    void BaseSliderWidgetQt::updateFromSpinBox() {
        const double newValue = spinBox_->value();
        if (fabs(newValue - spinnerValue_) > std::numeric_limits<double>::epsilon()) {
            spinnerValue_ = newValue;
            newSpinnerValue(spinnerValue_);
            updateSlider();
            emit valueChanged();
        }
    }

    void BaseSliderWidgetQt::updateSpinBox() {
        QSignalBlocker spinBlock(spinBox_);

        spinnerValue_ = transformValueToSpinner();
        spinBox_->setValue(spinnerValue_);
    }

    void BaseSliderWidgetQt::updateSlider() {
        QSignalBlocker slideBlock(slider_);

        sliderValue_ = transformValueToSlider();
        updateOutOfBounds();

        slider_->setValue(sliderValue_);
    }

    void BaseSliderWidgetQt::updateOutOfBounds() {
        bool isOutOfBounds = (slider_->maximum() < sliderValue_ || slider_->minimum() > sliderValue_);
        if (isOutOfBounds != slider_->property("outOfBounds").toBool()) {
            slider_->setProperty("outOfBounds", isOutOfBounds);
            slider_->style()->unpolish(slider_);
            slider_->style()->polish(slider_);
        }
    }

    bool BaseSliderWidgetQt::eventFilter(QObject* watched, QEvent* event) {
        if (watched == slider_ && event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent* me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton && !slider_->isSliderDown() && isEnabled()) {
                auto newPos = slider_->minimum() + static_cast<double>(me->pos().x()) *
                    (slider_->maximum() - slider_->minimum()) /
                    slider_->width();
                slider_->setValue(static_cast<int>(newPos));
                me->accept();
                return true;
            }
        }

        return QObject::eventFilter(watched, event);
    }


}