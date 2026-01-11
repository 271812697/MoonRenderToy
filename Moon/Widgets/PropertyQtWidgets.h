#pragma once
#include <QWidget>
namespace MOON {
class Property;
class PropertyQtWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PropertyQtWidget(QWidget* parent ,Property* prop);
protected:
    Property* mProps;
};

}
