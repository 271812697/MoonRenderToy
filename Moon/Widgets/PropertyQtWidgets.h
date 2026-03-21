#pragma once
#include <QWidget>
#include <QVariant>
namespace MOON {
class WidgetProperty;
class PropertyQtWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PropertyQtWidget(QWidget* parent , WidgetProperty* prop);
    PropertyQtWidget(QWidget* parent);
    void setProp(WidgetProperty*prop);
    virtual QVariant widgetValue()=0;
    virtual void setWidgetValue(const QVariant& value)=0;
    virtual void OnValueChanged();
protected:
    WidgetProperty* mProps;
};

}
