# 草图工具逐个解析（DrawSketchHandler 家族）

> 框架级机制（事件层/坐标层/状态机/模板方法/提交）请先读
> [SketchModelingWidget.md](../SketchModelingWidget.md) 与
> [SketchModelingWidgetArchitecture.md](../SketchModelingWidgetArchitecture.md)。
> 本目录是每个具体工具的专项文档。

## 工具总览

| 工具 | 继承 | 状态机 | 构造方式 | 文档 |
| --- | --- | --- | --- | --- |
| 点 Point | DefaultHandler | OneSeekEnd | — | [DrawSketchHandlerPoint.md](./DrawSketchHandlerPoint.md) |
| 直线 Line | DefaultHandler | TwoSeekEnd | LineConstructionMethod（未实际接线） | [DrawSketchHandlerLine.md](./DrawSketchHandlerLine.md) |
| 多段线 LineSet | **直接继承 DrawSketchHandler** | 自建 4 态 | 线段/圆弧 × 自由/相切/垂直 | [DrawSketchHandlerLineSet.md](./DrawSketchHandlerLineSet.md) |
| 圆 Circle | DefaultHandler | ThreeSeekEnd（Center 会跳步） | CircleEllipseConstructionMethod | [DrawSketchHandlerCircle.md](./DrawSketchHandlerCircle.md) |
| 圆弧 Arc | DefaultHandler | ThreeSeekEnd | CircleEllipseConstructionMethod | 见主文档 |
| 样条 BSpline | DefaultHandler | ThreeSeekEnd（停留 SeekSecond） | BSplineConstructionMethod | [DrawSketchHandlerBSpline.md](./DrawSketchHandlerBSpline.md) |
| 倒角/圆角 Fillet | DefaultHandler | TwoSeekEnd | FilletConstructionMethod | [DrawSketchHandlerFillet.md](./DrawSketchHandlerFillet.md) |
| 矩形 Rectangle | DefaultHandler | FiveSeekEnd（按方式跳步） | RectangleConstructionMethod | [DrawSketchHandlerRectangle.md](./DrawSketchHandlerRectangle.md) |
| 旋转 Rotate | DefaultHandler | ThreeSeekEnd | — | [DrawSketchHandlerRotate.md](./DrawSketchHandlerRotate.md) |
| 对称 Symmetry | DefaultHandler | OneSeekEnd | — | [DrawSketchHandlerSymmetry.md](./DrawSketchHandlerSymmetry.md) |
| 裁剪 Trimming | **直接继承 DrawSketchHandler** | 无 | — | [DrawSketchHandlerTrimming.md](./DrawSketchHandlerTrimming.md) |

## 三类实现风格

1. **走模板的“几何生成型”**（Point/Line/Circle/Arc/Rectangle/Rotate/Symmetry/BSpline）：
   点击累积数据 → `createShape` 生成 `ShapeGeometry` → `End` 后由 `executeCommands` 提交；
2. **覆盖提交的“操作已有几何型”**（Fillet/Symmetry/Rotate）：不（只）生成新几何，而是对
   已有草图几何做 fillet/对称/旋转，通常重写 `executeCommands` 直接调用 `SketcherObj` 的操作；
3. **不依赖模板的自定义型**（LineSet/Trimming）：多段线的“续接上一段”语义与裁剪的“无状态
   连续操作”语义不适合三点击状态机，直接继承 `DrawSketchHandler` 自建流程。

## 阅读建议

先读 Point（最简模板用法）→ Line（两点 + 几何生成）→ Circle/Arc（构造方式 + 跳步）→
BSpline（拖拽编辑热点）→ Rectangle（多状态 + 多方式）→ Rotate/Symmetry/Fillet（操作已有几何）→
LineSet/Trimming（自建流程）。
