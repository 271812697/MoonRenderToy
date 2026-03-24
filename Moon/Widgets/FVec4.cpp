#include "Widgets/FVec4.h"
#include "Widgets/Property.h"
namespace MOON {
Fvec4::Fvec4(QWidget* parent, WidgetProperty* prop) : PropertyQtWidget(parent,prop)
{
    m_spinX = new QDoubleSpinBox(this);
    m_spinY = new QDoubleSpinBox(this);
    m_spinZ = new QDoubleSpinBox(this);
    m_spinW = new QDoubleSpinBox(this);

    // ========== 【Vec3 浮点配置 核心优化 - 必改】 ==========
    // 1. 设置浮点数值范围（根据你的业务需求调整，比如3D坐标一般-9999~9999足够）
    m_spinX->setRange(-9999.99f, 9999.99f);
    m_spinY->setRange(-9999.99f, 9999.99f);
    m_spinZ->setRange(-9999.99f, 9999.99f);
    m_spinW->setRange(-9999.99f, 9999.99f);

    // 2. 设置小数位数【重中之重】：Vec3推荐保留2位小数，平衡精度和显示简洁
    //    可按需修改：1位(0.1)、3位(0.001)，最多支持15位
    m_spinX->setDecimals(2);
    m_spinY->setDecimals(2);
    m_spinZ->setDecimals(2);
    m_spinW->setDecimals(2);

    // 3. 设置【单步调整值】：点击上下箭头时，每次增减0.1，符合浮点微调习惯
    m_spinX->setSingleStep(0.1);
    m_spinY->setSingleStep(0.1);
    m_spinZ->setSingleStep(0.1);
    m_spinW->setSingleStep(0.1);

    // ========== 【单元格嵌入优化 - 必加，解决显示拥挤问题】 ==========
    // 去掉上下箭头按钮，节省宽度，让3个控件在单元格里完美并排显示
    m_spinX->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_spinY->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_spinZ->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_spinW->setButtonSymbols(QAbstractSpinBox::NoButtons);

    // 自适应拉伸，3个控件均分单元格宽度，不会有空白/溢出
    m_spinX->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_spinY->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_spinZ->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_spinW->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // ========== 布局优化 ==========
   
    QGridLayout* hLayout = new QGridLayout(this);
   
    hLayout->setContentsMargins(1, 0, 1, 0); // 极小内边距，贴合单元格
    hLayout->setSpacing(3);                  // 控件间距适中
    hLayout->addWidget(m_spinX,0,0);
    hLayout->addWidget(m_spinY,0,1);
    hLayout->addWidget(m_spinZ,1,0);
    hLayout->addWidget(m_spinW,1,1);

    this->setLayout(hLayout);
    //m_spinX;
    //&QDoubleSpinBox::valueChanged(double);
    // 正确写法，推荐！
    QObject::connect(m_spinX, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        this, &Fvec4::onValueChange);
    QObject::connect(m_spinY, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        this, &Fvec4::onValueChange);
    QObject::connect(m_spinZ, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        this, &Fvec4::onValueChange);
    QObject::connect(m_spinW, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        this, &Fvec4::onValueChange);
}

Fvec4::Fvec4(QWidget* parent):Fvec4::Fvec4(parent, nullptr)
{
}

void Fvec4::setVec4Value(float x, float y, float z,float w)
{
    m_spinX->setValue(x);
    m_spinY->setValue(y);
    m_spinZ->setValue(z);
    m_spinW->setValue(w);
}

// 以下所有函数完全不变，你的原代码是正确的
void Fvec4::setVec4Value(const Maths::FVector4& vec)
{
    setVec4Value(vec.x, vec.y, vec.z,vec.w);
}
void Fvec4::setWidgetValue(const QVariant& value) {
    setVec4Value(value.value<Maths::FVector4>());
}
QVariant Fvec4::widgetValue() {
    return QVariant::fromValue(getVec4Value());
 }

Maths::FVector4 Fvec4::getVec4Value() const
{
    return Maths::FVector4(m_spinX->value(), m_spinY->value(), m_spinZ->value(),m_spinW->value()); // float值直接构造QVector3D，完美适配
}

void Fvec4::onValueChange(double val) {
    OnValueChanged();
}


}
