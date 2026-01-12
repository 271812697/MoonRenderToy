#pragma once
#include <QWidget>
namespace MOON {
class Property;
class PropertyQtWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PropertyQtWidget(QWidget* parent ,Property* prop);
    PropertyQtWidget(QWidget* parent);
    void setProp(Property*prop);
    virtual void OnValueChanged();
protected:
    Property* mProps;
};

}
