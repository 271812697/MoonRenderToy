# DrawSketchHandlerCircle：圆工具

## 1. 概述

与圆弧工具共享 `CircleEllipseConstructionMethod { Center, ThreeRim, End }`：

- **Center（圆心式）**：点 1 圆心、点 2 半径 → 直接提交（跳过第三点）；
- **ThreeRim（三点式）**：点 1/点 2/点 3 都在圆周上，三点求外接圆。

```cpp
class DrawSketchHandlerCircle : public DrawSketchDefaultHandler<DrawSketchHandlerCircle,
                                                                 StateMachines::ThreeSeekEnd, 3,
                                                                 CircleEllipseConstructionMethod>
```

## 2. 状态推进的特殊之处：跳步

Circle 覆写了 `onButtonPressed`，按构造方式跳过不需要的状态：

```cpp
void onButtonPressed(Base::Vector2d pos) {
    this->updateDataAndDrawToPosition(pos);
    if (canGoToNextMode()) {
        if (state() == SeekSecond && constructionMethod() == Center) {
            setState(SelectMode::End);          // 圆心式第 2 点即完成，跳过 SeekThird
        } else {
            moveToNextMode();                   // 三点式正常推进
        }
    }
}
```

## 3. 三种状态下捕获的数据

| 状态 | Center | ThreeRim |
| --- | --- | --- |
| SeekFirst | `centerPoint = pos` | `firstPoint = pos` |
| SeekSecond | `secondPoint = pos; radius = |pos − center|` | `centerPoint = (pos − firstPoint)/2 + firstPoint`（临时中点）；`radius = |pos − centerPoint|` |
| SeekThird | 不进入 | `areCollinear` 守卫 → `Part::Geom2dCircle::getCircleCenter(p1,p2,p3)` 求外心 → `radius` |

> 与圆弧不同，圆的三点求心用的是 `Part::Geom2dCircle::getCircleCenter`（FreeCAD 几何类），
> 圆弧工具用的是文件内自带的 `getCircleCenter` 局部实现。

## 4. 预览与提交

```cpp
void createShape(bool) {
    ShapeGeometry.clear();
    if (radius < Precision::Confusion()) return;
    addCircleToShapeGeometry(toVector3d(centerPoint), radius, true);
}
```

`onUpdate` 在 `SeekSecond` 时额外画半径数值浮标：

```cpp
if (state() == SeekSecond) drawFloatValue(m_internal->radius);
```

## 5. 注意点

- `canGoToNextMode` 只拦 `SeekSecond` 的零半径，`SeekThird` 的共线由 `updateData...` 里的
  `areCollinear` + try/catch 兜底；
- 成员 `isDiameter` 未使用；
- `Center` 方式下 `SeekThird` 永远到不了，`createShape` 的 `SeekThird` 分支自然也不会执行。
