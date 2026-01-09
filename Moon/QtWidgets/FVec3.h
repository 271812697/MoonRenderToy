#pragma once
#include <Maths/FVector3.h>
#include <QWidget>
#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <QVector3D>

class Fvec3 : public QWidget
{
    Q_OBJECT
public:
    explicit Fvec3(QWidget* parent = nullptr);

    void setVec3Value(float x, float y, float z);
    void setVec3Value(const Maths::FVector3& vec);
    Maths::FVector3 getVec3Value() const;
    float x() const;
    float y() const;
    float z() const;

private:
    QDoubleSpinBox* m_spinX;
    QDoubleSpinBox* m_spinY;
    QDoubleSpinBox* m_spinZ;
};
