# DrawSketchHandlerTrimming：裁剪工具

## 1. 概述

**不依赖模板**，直接继承 `DrawSketchHandler`。没有状态机：鼠标悬停即实时计算“要剪哪条线、
剪到哪两个交点”，每次左键点击执行一次裁剪，可连续裁剪。

```cpp
class DrawSketchHandlerTrimming : public DrawSketchHandler
```

## 2. 关键数据

```cpp
Base::Vector2d a, b;      // 裁剪区间的两个交点（预览用）
double u1, u2;            // 被裁曲线上的参数
int trimCurveId;          // 当前悬停命中的曲线 id
```

## 3. 每帧裁剪数据更新

```cpp
void onMouseMove() {
    DrawSketchHandler::onMouseMove();   // 刷新 onSketchPos
    updateTrimData();
}

void updateTrimData() {
    int GeoId = sketchObj->getPickGeoIndex(onSketchPos, 视口矩阵);   // 屏幕空间拾取曲线
    if (GeoId != -1 && sketchObj->seekTrimPoints(GeoId, 光标3D点,
            GeoId1, intersect1, GeoId2, intersect2, u1, u2)) {
        a = intersect1; b = intersect2; trimCurveId = GeoId;
    } else {
        trimCurveId = -1;
    }
}
```

`seekTrimPoints` 找到被裁曲线与两侧相交曲线的交点；找不到就清空命中。

## 4. 预览与执行

```cpp
// 预览：命中时画两个洋红交点
if (trimCurveId != -1) {
    pushColor(洋红); drawPoint(a); drawPoint(b); popColor();
}

// 执行
void onLeftMousePressed() {
    updateTrimData();
    if (trimCurveId != -1)
        sketchObj->trim(trimCurveId, u1, u2, a3D, b3D);
}
```

## 5. 注意点

- 无状态机、无 `ShapeGeometry`、无预览折线——预览只有两个交点；
- 没有覆写右键/Esc：右键默认空实现，**退出只能靠工具栏取消勾选**
  （`GizmoRenderPass` 里创建时已 `setActive(false)`，由 `SketchToolbar` 控制激活）；
- `getPickGeoIndex` 与 `seekTrimPoints` 的几何正确性都在 `SketcherObj` 一侧。
