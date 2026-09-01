# DrawSketchHandlerFillet：圆角/倒角工具

## 1. 概述

对**已有草图几何**做圆角（Fillet）或倒角（Chamfer）：连续选中两条有界曲线（当前只支持直线），
提交时调用 `SketcherObj::fillet`。

```cpp
class DrawSketchHandlerFillet : public DrawSketchDefaultHandler<DrawSketchHandlerFillet,
                                                                 StateMachines::TwoSeekEnd, 0,
                                                                 FilletConstructionMethod>
```

## 2. 与“几何生成型”工具的关键区别

**它不生成预览几何**：没有覆写 `createShape`，基类的 `createShape` 是空实现，`ShapeGeometry`
始终为空。交互数据是两条曲线的 id 与拾取点：

```cpp
int geoId1, geoId2;
Base::Vector2d firstPos, secondPos;
bool preserveCorner = true;
```

## 3. 拾取流程

```cpp
// SeekFirst
geoId1 = getPreselectCurve();   // 悬停命中的曲线 id
firstPos = onSketchPos;
// SeekSecond
geoId2 = getPreselectCurve();
secondPos = onSketchPos;
```

`canGoToNextMode` 校验并选中：

```cpp
if (state() == SeekFirst && geoId1 >= 0) {
    if (obj->getGeometry(geoId1)->isDerivedFrom<Part::GeomBoundedCurve>()) {
        obj->addSelect(geoId1);
        return true;
    }
}
if (state() == SeekSecond && geoId2 >= 0 && geoId2 != geoId1) { /* 同理 */ }
```

## 4. 提交：直接操作草图

```cpp
void executeCommands() override {
    double radius = 0;
    if (两条都是 GeomLineSegment) {
        radius = Part::suggestFilletRadius(line1, line2, refPnt1, refPnt2);
        if (radius < 0) return;
    }
    obj->fillet(geoId1, geoId2, firstPos3D, secondPos3D, radius, true, preserveCorner, isChamfer);
    obj->removeSelect({geoId1, geoId2});
}
```

`isChamfer = constructionMethod() == Chamfer`，由 `M` 键切换。

## 5. 注意点

- 不经过 `ShapeGeometry`/`addGeometry`，走的是 `SketcherObj::fillet` 的专用入口；
- 圆角半径由 `suggestFilletRadius` 根据拾取点自动推算，交互里没有半径输入；
- `onUpdate` 里画两个拾取点的代码被注释，当前无任何预览反馈（只能看到选中高亮）；
- 错误用 `CORE_ERROR` 记录并吞掉，不中断工具。
