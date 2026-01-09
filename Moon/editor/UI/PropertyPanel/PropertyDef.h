#pragma once
#include <QVariant>
namespace MOON {
    // 组件属性结构体
    struct ComponentProperty
    {
        QString name;       // 属性名
        QVariant value;     // 属性值
        QVariant::Type type;// 属性类型
    };
    // 关键：注册ComponentProperty为Qt元类型
    Q_DECLARE_METATYPE(ComponentProperty)
    // 组件结构体
    struct Component
    {
        QString name;                   // 组件名
        QList<ComponentProperty> properties; // 组件属性列表
    };
    // 关键：注册Component为Qt元类型
    Q_DECLARE_METATYPE(Component)
}