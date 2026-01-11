#include "Widgets/FVec3.h"
#include "editor/UI/PropertyPanel/Property.h"
namespace MOON {
Fvec3::Fvec3(QWidget* parent, Property* prop) : PropertyQtWidget(parent,prop)
{
    m_spinX = new QDoubleSpinBox(this);
    m_spinY = new QDoubleSpinBox(this);
    m_spinZ = new QDoubleSpinBox(this);

    // ========== 【Vec3 浮点配置 核心优化 - 必改】 ==========
    // 1. 设置浮点数值范围（根据你的业务需求调整，比如3D坐标一般-9999~9999足够）
    m_spinX->setRange(-9999.99f, 9999.99f);
    m_spinY->setRange(-9999.99f, 9999.99f);
    m_spinZ->setRange(-9999.99f, 9999.99f);

    // 2. 设置小数位数【重中之重】：Vec3推荐保留2位小数，平衡精度和显示简洁
    //    可按需修改：1位(0.1)、3位(0.001)，最多支持15位
    m_spinX->setDecimals(2);
    m_spinY->setDecimals(2);
    m_spinZ->setDecimals(2);

    // 3. 设置【单步调整值】：点击上下箭头时，每次增减0.1，符合浮点微调习惯
    m_spinX->setSingleStep(0.1);
    m_spinY->setSingleStep(0.1);
    m_spinZ->setSingleStep(0.1);

    // ========== 【单元格嵌入优化 - 必加，解决显示拥挤问题】 ==========
    // 去掉上下箭头按钮，节省宽度，让3个控件在单元格里完美并排显示
    m_spinX->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_spinY->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_spinZ->setButtonSymbols(QAbstractSpinBox::NoButtons);

    // 自适应拉伸，3个控件均分单元格宽度，不会有空白/溢出
    m_spinX->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_spinY->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_spinZ->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // ========== 布局优化 ==========
    QHBoxLayout* hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(1, 0, 1, 0); // 极小内边距，贴合单元格
    hLayout->setSpacing(3);                  // 控件间距适中
    hLayout->addWidget(m_spinX);
    hLayout->addWidget(m_spinY);
    hLayout->addWidget(m_spinZ);

    this->setLayout(hLayout);
    //m_spinX;
    //&QDoubleSpinBox::valueChanged(double);
    // 正确写法，推荐！
    QObject::connect(m_spinX, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        this, &Fvec3::onValueChange);
    QObject::connect(m_spinY, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        this, &Fvec3::onValueChange);
    QObject::connect(m_spinZ, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        this, &Fvec3::onValueChange);
}

Fvec3::Fvec3(QWidget* parent):Fvec3::Fvec3(parent, nullptr)
{
}

void Fvec3::setVec3Value(float x, float y, float z)
{
    m_spinX->setValue(x);
    m_spinY->setValue(y);
    m_spinZ->setValue(z);
}

// 以下所有函数完全不变，你的原代码是正确的
void Fvec3::setVec3Value(const Maths::FVector3& vec)
{
    setVec3Value(vec.x, vec.y, vec.z);
}

Maths::FVector3 Fvec3::getVec3Value() const
{
    return Maths::FVector3(m_spinX->value(), m_spinY->value(), m_spinZ->value()); // float值直接构造QVector3D，完美适配
}
void Fvec3::onValueChange(double val) {
    //mProps->setPropertyValue(QVariant::fromValue(getVec3Value()));
    mProps->onPropertyValueChange();
}


}
