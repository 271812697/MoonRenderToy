#pragma once
#include "Widgets/PropertyQtWidgets.h"
#include "Widgets/checkbox.h"
namespace MOON {
    class  SliderCheckBox: public PropertyQtWidget
    {
        Q_OBJECT
    public:
        SliderCheckBox(QWidget* parent, WidgetProperty* prop);
        bool getValue()const;
        void setValue(bool val);
        virtual QVariant widgetValue() override;
        virtual void setWidgetValue(const QVariant& value) override;
    public Q_SLOTS:
        void toggle(bool);

    private:
		SlidingCheckBox* m_checkBox;
    };

}