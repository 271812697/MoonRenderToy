#pragma once
#include "Widgets/PropertyQtWidgets.h"
#include "Widgets/numberwidget.h"
#include <Maths/FVector3.h>
#include <QWidget>
#include <QHBoxLayout>
#include <QVector3D>
namespace MOON {
Q_DECLARE_METATYPE(Maths::FVector3);

class Fvec3 : public PropertyQtWidget
{
    Q_OBJECT
public:
    explicit Fvec3(QWidget* parent , WidgetProperty* prop);
    explicit Fvec3(QWidget* parent);
    void setVec3Value(float x, float y, float z);
    void setVec3Value(const Maths::FVector3& vec);
    Maths::FVector3 getVec3Value() const;
    virtual QVariant widgetValue()override;
    virtual void setWidgetValue(const QVariant& value) override;
    float x() const;
    float y() const;
    float z() const;

public Q_SLOTS:
    void onValueChange();
private:
    NumberWidget* m_spinX;
    NumberWidget* m_spinY;
    NumberWidget* m_spinZ;
};

}
