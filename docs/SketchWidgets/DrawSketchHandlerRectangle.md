# DrawSketchHandlerRectangle：矩形工具

## 1. 概述

最复杂的模板工具（FiveSeekEnd + 4 种构造方式 + 圆角/边框分支）。当前默认状态只走
“普通矩形”路径。

```cpp
class DrawSketchHandlerRectangle : public DrawSketchDefaultHandler<DrawSketchHandlerRectangle,
                                                                    StateMachines::FiveSeekEnd, 3,
                                                                    RectangleConstructionMethod>
```

## 2. 构造方式与状态流

```cpp
enum class RectangleConstructionMethod { Diagonal, CenterAndCorner, ThreePoints, CenterAnd3Points, End };
```

| 方式 | 点 1 | 点 2 | 点 3 | 后续 |
| --- | --- | --- | --- | --- |
| Diagonal 对角点 | corner1 | corner3（对角） | 圆角/厚度（可选） | — |
| CenterAndCorner 中心+角 | center | corner3（角） | 圆角/厚度（可选） | — |
| ThreePoints 三点 | corner1 | corner2（边长） | corner3（定宽方向/角度） | — |
| CenterAnd3Points 中心+三点 | center | corner1 | corner2（定角） | — |

矩形必须保证角点 CCW 环绕，代码里用 `corner3−corner1` 的符号判断并交换 corner2/corner4
（`cornersReversed` 记录是否交换）。`ThreePoints` 类方法还会计算 `angle123/angle412`
（相邻角），供非直角/圆角计算使用。

## 3. 跳步规则（onButtonPressed）

```cpp
if (Diagonal || CenterAndCorner) {
    SeekSecond && !roundCorners && !makeFrame → End
    (SeekThird && roundCorners && !makeFrame) || (SeekThird && !roundCorners && makeFrame) → End
    SeekFourth → End
    否则 moveToNextMode()
} else { /* ThreePoints/CenterAnd3Points 同理，把上面的 SeekSecond 换成 SeekThird */ }
```

即普通矩形 2 次点击，圆角矩形 3 次（第 3 次定半径），带边框 3~4 次（定厚度）。
`roundCorners / makeFrame` 目前没有入口置位（恒 false），所以实际只有 2 点对角/中心式生效。

## 4. 几何生成

`createShape`：

```cpp
vecL = corner2 − corner1;  vecW = corner4 − corner1;
length/width/angle 计算；长度或宽度为 0 或角为 0/π 则放弃
→ createFirstRectangleGeometries(vecL, vecW, L1, L2)   // 4 条边线
→ makeFrame && thickness 非零 → 第二层矩形（当前被注释）
→ !onlyeditoutline → finish* 系列（加圆角弧、边框构造线、构造点）
```

注意：预览路径 `CreateAndDrawShapeGeometry` 恒传 `onlyeditoutline=true`，所以预览只有 4 条边；
而 `executeCommands` 没有覆写，提交的是**最后一份预览几何**（4 条边）。`finish*` 系列（圆角、
边框、构造点）依赖 `onlyeditoutline=false` 的调用，当前链路里**不会执行**——与 FreeCAD 原版
的差异点。

## 5. 注意点

- 大量 `finish*` 辅助函数里的约束代码（Coincident/Perpendicular/Symmetric）被注释，几何
  能画出来但没有任何约束；
- `calculateRadius/calculateThickness` 只有 `SeekThird/SeekFourth/SeekFifth` 触发，普通矩形
  到不了；
- `lengthSign/widthSign` 为 OVP 修正残留。
