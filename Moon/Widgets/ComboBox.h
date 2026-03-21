#pragma once
#include "Widgets/PropertyQtWidgets.h"
#include <QComboBox>
namespace MOON {
    class  ComboBox: public PropertyQtWidget
    {
        Q_OBJECT
    public:
        ComboBox(QWidget* parent, WidgetProperty* prop);
        void addComboList(const QList<QString>&list);
        int getCurrentIndex();
        virtual QVariant widgetValue()override;
        virtual void setWidgetValue(const QVariant& value) override;
    private slots:
        void onProvinceSelect(const QString& selectText);
    private:
        QComboBox* myCbox;
    };

}