# DrawSketchHandlerRotate：旋转工具

## 1. 概述

把**当前选中的草图几何**绕一个中心旋转（可多份拷贝）。与 Symmetry 一样，它操作的是已有几何。

```cpp
class DrawSketchHandlerRotate : public DrawSketchDefaultHandler<DrawSketchHandlerRotate,
                                                                 StateMachines::ThreeSeekEnd, 0>
```

## 2. 数据与三次点击

```cpp
std::vector<int> listOfGeoIds;      // 每帧从 getSelectIds() 刷新
Base::Vector2d centerPoint, startPoint, endPoint;
double length, startAngle, endAngle, totalAngle, individualAngle;
int numberOfCopies;                // A/S 调整，默认 0
bool deleteOriginal;               // copies==0 时旋转并删除原件
```

```
SeekFirst : centerPoint = pos
SeekSecond: length/startAngle/startPoint（画中心→起点的辅助线）
SeekThird : endAngle；totalAngle 用与圆弧 Center 相同的“连续性”公式选顺/逆
```

## 3. 拷贝数与预览

`createShape`：

```cpp
numberOfCopiesToMake = (numberOfCopies == 0) ? 1 : numberOfCopies;  // 默认 1 份
deleteOriginal = (numberOfCopies == 0);
individualAngle = totalAngle / numberOfCopiesToMake;
for (i = 1..n) for (geoId : listOfGeoIds) {
    copy 几何 → 绕 center 旋转 individualAngle*i → 加入 ShapeGeometry
}
if (onlyeditoutline) 再画中心→起点/终点的两条辅助线
```

`SeekSecond` 阶段只画中心→起点的预览线。

## 4. 提交

```cpp
void executeCommands() override {
    createShape(false);            // 用最终参数重建（此时不加辅助线）
    SupperClass::executeCommands();// 把 ShapeGeometry 逐个 addGeometry
    if (deleteOriginal) deleteOriginalGeos();   // 删除原几何
}
```

## 5. 注意点

- 选择来源是 `SketcherObj::getSelectIds()`（拾取/树选中），`onUpdate` 每帧刷新；
- `A` 加一份、`S` 减一份（下限 0）；`canGoToNextMode` 只拦 0° 旋转；
- 旋转用的是 `Base::Matrix4D(center, Z轴, angle)` 变换矩阵。
