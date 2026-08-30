#include "Widgets/FVec3.h"
#include "Widgets/Property.h"
namespace MOON {
Fvec3::Fvec3(QWidget* parent, WidgetProperty* prop) : PropertyQtWidget(parent, prop) {
    m_spinX = new NumberWidget(this);
    m_spinY = new NumberWidget(this);
    m_spinZ = new NumberWidget(this);

    for (auto* spin : {m_spinX, m_spinY, m_spinZ}) {
        spin->setRange(-9999.99, 9999.99);
        spin->setIncrement(0.1);
        QSizePolicy sp = spin->sizePolicy();
        sp.setHorizontalPolicy(QSizePolicy::Expanding);
        spin->setSizePolicy(sp);
    }
    m_spinX->setPrefix("x");
    m_spinY->setPrefix("y");
    m_spinZ->setPrefix("z");

    QHBoxLayout* hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(2);
    hLayout->addWidget(m_spinX);
    hLayout->addWidget(m_spinY);
    hLayout->addWidget(m_spinZ);

    QObject::connect(m_spinX, &NumberWidget::valueChanged, this, &Fvec3::onValueChange);
    QObject::connect(m_spinY, &NumberWidget::valueChanged, this, &Fvec3::onValueChange);
    QObject::connect(m_spinZ, &NumberWidget::valueChanged, this, &Fvec3::onValueChange);
}

Fvec3::Fvec3(QWidget* parent) : Fvec3(parent, nullptr) {
}

void Fvec3::setVec3Value(float x, float y, float z) {
    m_spinX->initValue(x);
    m_spinY->initValue(y);
    m_spinZ->initValue(z);
}

void Fvec3::setVec3Value(const Maths::FVector3& vec) {
    setVec3Value(vec.x, vec.y, vec.z);
}

void Fvec3::setWidgetValue(const QVariant& value) {
    setVec3Value(value.value<Maths::FVector3>());
}

QVariant Fvec3::widgetValue() {
    return QVariant::fromValue(getVec3Value());
}

Maths::FVector3 Fvec3::getVec3Value() const {
    return Maths::FVector3(static_cast<float>(m_spinX->getValue()),
                           static_cast<float>(m_spinY->getValue()),
                           static_cast<float>(m_spinZ->getValue()));
}

float Fvec3::x() const {
    return static_cast<float>(m_spinX->getValue());
}

float Fvec3::y() const {
    return static_cast<float>(m_spinY->getValue());
}

float Fvec3::z() const {
    return static_cast<float>(m_spinZ->getValue());
}

void Fvec3::onValueChange() {
    OnValueChanged();
}

}
