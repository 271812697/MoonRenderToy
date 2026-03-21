#pragma once
#include "Widgets/PropertyQtWidgets.h"
#include "Widgets/utils.h"   
#include <QObject>  // for Q_OBJECT, signals
#include <QWidget>  // for QWidget

class QEvent;
class QSlider;
namespace MOON {
    class NumberLineEdit;
    template <typename T>
    class OrdinalBaseWidget {
    public:
        virtual ~OrdinalBaseWidget() = default;

        virtual T getValue() const = 0;
        virtual void setValue(T value) = 0;
        virtual void initValue(T value) = 0;
        virtual void setMinValue(T minValue) = 0;
        virtual void setMaxValue(T maxValue) = 0;
        virtual void setIncrement(T increment) = 0;
    };

    class  BaseSliderWidgetQt : public PropertyQtWidget {
        Q_OBJECT
    public:
        BaseSliderWidgetQt(QWidget* parent,bool intMode = false);
        virtual ~BaseSliderWidgetQt() = default;

        void setWrapping(bool wrap);
        bool wrapping() const;

    protected:
        virtual double transformValueToSpinner() = 0;
        virtual double transformMinValueToSpinner() = 0;
        virtual double transformMaxValueToSpinner() = 0;
        virtual double transformIncrementToSpinner() = 0;

        virtual int transformValueToSlider() = 0;
        virtual int transformMinValueToSlider() = 0;
        virtual int transformMaxValueToSlider() = 0;
        virtual int transformIncrementToSlider() = 0;

        virtual int transformIncrementToSpinnerDecimals() = 0;

        virtual void newSliderValue(int val) = 0;
        virtual void newSpinnerValue(double val) = 0;

        void applyInit();
        void applyValue();
        void applyMinValue();
        void applyMaxValue();
        void applyIncrement();

        static constexpr int sliderMax_ = 10000;

    signals:
        void valueChanged();

    private:
        void updateFromSlider();
        void updateFromSpinBox();

        /**
         * \brief updates the value of the spin box from the slider value
         */
        void updateSpinBox();
        /**
         * \brief updates the value of the slider from the spin box value
         */
        void updateSlider();

        void updateOutOfBounds();

        virtual bool eventFilter(QObject* watched, QEvent* event) override;

        NumberLineEdit* spinBox_;
        QSlider* slider_;
        double spinnerValue_;
        int sliderValue_;

    protected:
        //ConstraintBehavior minCB_;
        //ConstraintBehavior maxCB_;
    };
    template <typename T>
    class SliderWidgetQt final : public BaseSliderWidgetQt, public OrdinalBaseWidget<T> {
    public:
        SliderWidgetQt(QWidget* parent)
            : BaseSliderWidgetQt(parent,!std::is_floating_point_v<T>)
            , value_(0)
            , minValue_(0)
            , maxValue_(0)
            , increment_(0) {
        }
        virtual ~SliderWidgetQt() = default;

        // Implements OrdinalBaseWidget<T>
        virtual T getValue() const override;
        virtual void setValue(T value) override;
        virtual void initValue(T value) override;
        virtual void setMinValue(T minValue) override;
        virtual void setMaxValue(T maxValue) override;
        virtual void setIncrement(T increment) override;
        virtual QVariant widgetValue()override;
        virtual void setWidgetValue(const QVariant& value) override;

    protected:
        // Define the transforms
        T sliderToRepr(int val) const;
        int reprToSlider(T val) const;
        T spinnerToRepr(double val) const;
        double reprToSpinner(T val) const;

        virtual double transformValueToSpinner() override;
        virtual double transformMinValueToSpinner() override;
        virtual double transformMaxValueToSpinner() override;
        virtual double transformIncrementToSpinner() override;

        virtual int transformValueToSlider() override;
        virtual int transformMinValueToSlider() override;
        virtual int transformMaxValueToSlider() override;
        virtual int transformIncrementToSlider() override;

        virtual int transformIncrementToSpinnerDecimals() override;

        virtual void newSliderValue(int val) override;
        virtual void newSpinnerValue(double val) override;

        T value_;
        T minValue_;
        T maxValue_;
        T increment_;
    };

    using IntSliderWidgetQt = SliderWidgetQt<int>;
    using FloatSliderWidgetQt = SliderWidgetQt<float>;
    using DoubleSliderWidgetQt = SliderWidgetQt<double>;

    template <typename T>
    T SliderWidgetQt<T>::spinnerToRepr(double val) const {
        return static_cast<T>(val);
    }
    template <typename T>
    double SliderWidgetQt<T>::reprToSpinner(T val) const {
        return static_cast<double>(val);
    }
    template <typename T>
    double SliderWidgetQt<T>::transformValueToSpinner() {
        return reprToSpinner(value_);
    }
    template <typename T>
    double SliderWidgetQt<T>::transformMinValueToSpinner() {
        return reprToSpinner(minValue_);
    }
    template <typename T>
    double SliderWidgetQt<T>::transformMaxValueToSpinner() {
        return reprToSpinner(maxValue_);
    }
    template <typename T>
    double SliderWidgetQt<T>::transformIncrementToSpinner() {
        return reprToSpinner(increment_);
    }

    template <typename T>
    T SliderWidgetQt<T>::sliderToRepr(int val) const {
        if constexpr (std::is_floating_point_v<T>) {
            return this->minValue_ + (static_cast<T>(val) * (this->maxValue_ - this->minValue_) /
                static_cast<T>(this->sliderMax_));

        }
        else {
            return static_cast<T>(val);
        }
    }
    template <typename T>
    int SliderWidgetQt<T>::reprToSlider(T val) const {
        if constexpr (std::is_floating_point_v<T>) {
            if (this->maxValue_ == this->minValue_) return this->sliderMax_ / 2;
            const auto newVal =
                (val - this->minValue_) / (this->maxValue_ - this->minValue_) * this->sliderMax_;
            return static_cast<int>(std::clamp(newVal, T(std::numeric_limits<int>::lowest()),
                T(std::numeric_limits<int>::max())));
        }
        else {
            return static_cast<int>(val);
        }
    }
    template <typename T>
    int SliderWidgetQt<T>::transformValueToSlider() {
        return reprToSlider(value_);
    }
    template <typename T>
    int SliderWidgetQt<T>::transformMinValueToSlider() {
        if constexpr (std::is_floating_point_v<T>) {
            return 0;
        }
        else {
            return reprToSlider(minValue_);
        }
    }
    template <typename T>
    int SliderWidgetQt<T>::transformMaxValueToSlider() {
        if constexpr (std::is_floating_point_v<T>) {
            return this->sliderMax_;
        }
        else {
            return reprToSlider(maxValue_);
        }
    }
    template <typename T>
    int SliderWidgetQt<T>::transformIncrementToSlider() {
        return (increment_) / (this->maxValue_ - this->minValue_) * this->sliderMax_;
        //return reprToSlider(increment_);
    }

    template <typename T>
    int SliderWidgetQt<T>::transformIncrementToSpinnerDecimals() {
        return decimals(increment_);
    }

    template <typename T>
    void SliderWidgetQt<T>::newSpinnerValue(double val) {
        value_ = spinnerToRepr(val);
    }

    template <typename T>
    void SliderWidgetQt<T>::newSliderValue(int val) {
        value_ = sliderToRepr(val);
    }

    template <typename T>
    T SliderWidgetQt<T>::getValue() const {
        return value_;
    }

    template <typename T>
    void SliderWidgetQt<T>::setValue(T value) {
        if (value != value_) {
            value_ = value;
            applyValue();
        }
    }

    template <typename T>
    void SliderWidgetQt<T>::initValue(T value) {
        value_ = value;
        applyInit();
    }

    template <typename T>
    void SliderWidgetQt<T>::setMinValue(T minValue) {
        if (minValue_ != minValue ) {
            minValue_ = minValue;
      
            applyMinValue();
        }
    }

    template <typename T>
    void SliderWidgetQt<T>::setMaxValue(T maxValue) {
        if (maxValue_ != maxValue ) {
            maxValue_ = maxValue;
           
            applyMaxValue();
        }
    }

    template <typename T>
    void SliderWidgetQt<T>::setIncrement(T increment) {
        if (increment_ != increment) {
            increment_ = increment;
            applyIncrement();
        }
    }

    template<typename T>
    QVariant SliderWidgetQt<T>::widgetValue()
    {
        return QVariant(getValue());
    }

    template<typename T>
    void SliderWidgetQt<T>::setWidgetValue(const QVariant& value)
    {
        setValue(value.value<T>());
    }
 
}