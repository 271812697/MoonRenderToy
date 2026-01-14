#pragma once
#include "Widgets/PropertyQtWidgets.h"
#include <QComboBox>
namespace MOON {
    class  ComboBox: public PropertyQtWidget
    {
        Q_OBJECT
    public:
        ComboBox(QWidget* parent, Property* prop);
        void addComboList(const QList<QString>&list);
        int getCurrentIndex();
    private slots:
        void onProvinceSelect(const QString& selectText);
    private:
        QComboBox* myCbox;
    };

}