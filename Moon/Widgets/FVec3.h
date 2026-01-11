#pragma once
#include "Widgets/PropertyQtWidgets.h"
#include <Maths/FVector3.h>
#include <QWidget>
#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <QVector3D>
namespace MOON {
class Fvec3 : public PropertyQtWidget
{
    Q_OBJECT
public:
    explicit Fvec3(QWidget* parent , Property* prop);
    explicit Fvec3(QWidget* parent);
    void setVec3Value(float x, float y, float z);
    void setVec3Value(const Maths::FVector3& vec);
    Maths::FVector3 getVec3Value() const;
    float x() const;
    float y() const;
    float z() const;
public Q_SLOTS:
    void onValueChange(double val);
private:
    QDoubleSpinBox* m_spinX;
    QDoubleSpinBox* m_spinY;
    QDoubleSpinBox* m_spinZ;
};

}
