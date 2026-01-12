#pragma once
#include <memory>  // for unique_ptr

#include <QDoubleSpinBox>  // for QDoubleSpinBox
#include <QSize>           // for QSize
#include <QString>         // for QString
#include <QValidator>      // for QValidator, QValidator::State

class QEvent;
class QFocusEvent;
class QResizeEvent;
class QTimerEvent;
class QWheelEvent;
class QWidget;
namespace MOON {
    class NumberLineEditPrivate;
    class  NumberLineEdit : public QDoubleSpinBox {
    public:
        explicit NumberLineEdit(QWidget* parent = nullptr);
        explicit NumberLineEdit(bool intMode, QWidget* parent = nullptr);

        virtual ~NumberLineEdit() override;

        virtual QSize sizeHint() const override;

        virtual QSize minimumSizeHint() const override;

        // consider the current size of the widget in order to determine the best suitable number
        // representation, i.e. either regular floating point notation or scientific
        virtual QString textFromValue(double value) const override;
        virtual double valueFromText(const QString& str) const override;

        void setDecimals(int decimals);
        void setMinimum(double min);
        void setMaximum(double max);
        void setRange(double min, double max);
        /**
         * Sets the increment of a single step to @p inc.
         * If @p inc is zero, the spinbox buttons will be hidden.
         * @see QDoubleSpinBox::setSingleStep
         */
        void setIncrement(double inc);

        /**
         * \brief Overrides the timerEvent to prevent
         * spinbox to be updated twice in case of
         * calculations being slow
         */
        virtual void timerEvent(QTimerEvent* event) override;

        bool isValid() const;
        void setInvalid(bool invalid = true);

    protected:
        virtual QValidator::State validate(QString& text, int& pos) const override;
        virtual void focusInEvent(QFocusEvent* e) override;
        virtual void focusOutEvent(QFocusEvent* e) override;
        virtual void resizeEvent(QResizeEvent* e) override;
        virtual void changeEvent(QEvent* e) override;
        virtual void wheelEvent(QWheelEvent* e) override;

        virtual void stepBy(int steps) override;

    private:
        bool integerMode_;
        int minimumWidth_;
        QSize calcMinimumSize() const;

        QDoubleValidator* validator_;
        mutable QSize cachedMinimumSizeHint_;
        int visibleDecimals_ = 2;
        bool abbreviated_ = true;
        bool invalid_ = false;

        static std::unique_ptr<NumberLineEditPrivate> nlePrivate_;
    };

}