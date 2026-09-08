# 草图绘制：曲线、点与约束标注

> 本文讲“已经提交进 `SketcherObj` 的几何怎么画到视口”，即
> `SketcherObj::draw()` / `SketcherObjDraw.cpp` 这条路径。
> 正在创建的预览线（`DrawSketchHandler*` 的黄色即时预览）走的是另一条路径，见
> [SketchModelingWidget.md](SketchModelingWidget.md) 第 8 节。

## 1. 绘制入口与两套画布

`SketcherObj::draw()` 每帧会画两类内容：

1. **世界空间立即模式**（Im3D `renderer`）：背景网格/坐标轴、几何曲线、离散点；
2. **屏幕前景 ImDrawList**（`ImGui::GetForegroundDrawList()`）：约束图标、尺寸标注、
   约束序号等 2D 覆盖层。

两者共用一份几何数据：`mGeoSegment[geo]` 里的离散折线（`CurveSegment`）和关键点。

```text
SketcherObj::draw()
  ├─ drawBackground()            // 网格、无限 X/Y 轴、原点
  ├─ pushSize(curveLineWidth)
  ├─ 遍历 mGeoList：
  │     推 select/preselect/curve/construction 颜色
  │     实线画 mGeoSegment 折线；构造几何用虚线
  │     弹出颜色
  ├─ 再遍历 mGeoList：画关键点（保证点在线上层）
  ├─ 画选中点（最上层）
  ├─ drawConstraintLabels()      // 尺寸/角度等数值标注
  ├─ drawTangentIcons()          // 相切小图标
  └─ drawConstraintIcons()       // 重合/水平/垂直/平行/垂直约束图标等
```

关键文件：`Moon/Sketcher/SketcherObjDraw.cpp`。

## 2. 几何曲线怎么变成屏幕上的线

### 2.1 离散化缓存 `mGeoSegment`

每条几何加入 `mGeoList` 时，`addGeometry()` 会调用 `getCurveSegment()` 生成：

```text
CurveSegment {
    point[]    // OCC 曲线按参数等分 50 段得到的点（草图 2D 坐标）
    params[]   // 每个点对应的 OCC 参数 u
    sepoints[] // 关键点：线的 start/end；弧/圆的 start/end/center；...
}
```

绘制时把 `point[i]→point[i+1]` 用 `mPlane.valueEigen(...)` 抬到 3D 世界坐标，
再交给 Im3D `drawLine`。所以草图里的曲线本质是“50 段直线近似”，拾取、吸附、
绘制都基于同一份折线，误差一致。

### 2.2 颜色与选中态

颜色集中定义在 `SketcherObj::DrawOption`，按 ABGR 字节序（`{A,B,G,R}`）存放，
UI 面板（SketchTaskDialog）用 `ColorPickerProperty` 修改：

| 字段 | 含义 |
| --- | --- |
| `curveColor` | 普通曲线 |
| `constructionColor` | 构造几何（虚线 + 该颜色，默认绿色） |
| `pointColor` | 普通关键点 |
| `preselectColor` | 悬停高亮 |
| `selectColor` | 选中高亮 |
| `constraintColor` | 约束图标/序号颜色 |

绘制顺序决定优先级：先画普通颜色，再在**曲线循环里**判断选中/悬停覆盖颜色；
点标记单独循环（画在曲线上层）；选中点最后一层，保证不被其它点盖住。

### 2.3 构造几何虚线

判断“是否构造”同时看两个来源：

- `mConstructionGeoIds`（按 GeoId 维护的集合）；
- `Part::Geometry::getConstruction()`（几何体自带的构造标志，随 copy/clone 传播）。

构造曲线用 `drawDashedSketchPolyline()`：

```text
把折线逐段投影到屏幕像素坐标
按“实线 8px / 间隔 6px”切分
只把实线段转回世界坐标调用 drawLine
```

图案按屏幕像素计算，因此缩放相机时虚线疏密不变；段间相位连续，圆弧不会出现
“每小段重新起头”的麻点效果。

### 2.4 背景：网格与 X/Y 轴

`drawBackground()` 自适应网格：

- 从正交相机把视口高度换算成草图单位；
- 目标间距约 40px，再向上取整到 `1/2/5 × 10ⁿ` 的“好看”步长；
- 大格用深色、小格用浅色（`minorColor` 相关参数按 ABGR 传色）；
- X/Y 轴无限长，原点画一个点标记。

## 3. 约束与尺寸标注

### 3.1 两类标注：图标 vs 数值标签

| 类型 | 入口 | 内容 |
| --- | --- | --- |
| 非尺寸约束图标 | `drawConstraintIcons()` | Coincident/H/V/Parallel/Perpendicular/Equal/Symmetric/PointOnObject/Block 等 |
| 相切图标 | `drawTangentIcons()` | Tangent 的小切线符号（单独函数处理） |
| 尺寸/角度标签 | `drawConstraintLabels()` | Distance/Length/Radius/Diameter/Angle/DistanceX/Y 的箭头 + 数值框，可拖动 |

两类都先过滤 `isVisible == false` 与错误态：`constraintInError()` 为真时用红色绘制。

### 3.2 锚点计算 `anchorOf`

每个图标都要在草图元素上找一个“锚点”：

```text
PointPos != none → 取该几何对应点（start/end/mid）
PointPos == none → 取 getGeometryCenterSketch()：
    圆/椭圆取圆心；线取中点；其它曲线取离散折线中点
```

图标是否画两次取决于约束引用了几个对象：Equal/Perpendicular/Parallel/
两点 H/V 会把图标画在**每个对象自己的锚点**，并各自标**同一个约束序号**，
便于看出“哪几个元素属于同一条约束”。

### 3.3 各约束的图标样式（当前实现）

| 约束 | 图标 |
| --- | --- |
| Coincident | 偏移 16px 的小圆 + 圆内两个点 |
| PointOnObject | 锚点西北方向的半圆弧 + 中点小点 |
| Horizontal（两点/线） | 每个锚点上方一条水平横线 + 序号 |
| Vertical（两点/线） | 每个锚点右侧一条竖直短线 + 序号 |
| Parallel | 每个锚点一组 `//` + 序号 |
| Perpendicular | 每个锚点一个直角符号 + 序号 |
| Equal | 每个锚点一组 `=` + 序号 |
| Symmetric | 中点竖线 + 左右两点 |
| Block | 锚点处小方框 |

序号 = 约束在 `mConstraintList` 里的下标（视口内与列表同序），绘制为放大 1.4 倍
的屏幕文字，完整图标颜色跟随 `DrawOption::constraintColor`。

### 3.4 尺寸标注的轴对齐/自由轨道

`drawConstraintLabels()` 内部先算“测量轨道”再放数值框：

- `DistanceX`：两点之间水平轨道，数值放上方；
- `DistanceY`：两点之间竖直轨道，数值放右侧；
- 普通长度/半径/直径：沿被测量方向法线偏移，轨道位置跟随“默认标签偏移”，
  也可以被用户拖动（`m_labelManualOffsetPx` 记录手动偏移）。
- 角度：画一段圆弧轨道，标签可沿弧拖动。

轨道全部换算成屏幕像素距离，保证缩放后标签与曲线保持固定间距。

## 4. 交互相关绘制细节

### 4.1 尺寸标签可被拾取

`pickConstraintLabelAt()` 会把鼠标点与每个标签包围盒比较；命中的标签在悬停时
描边高亮，左键按住进入 `m_labelDrag`，拖动只改标签的屏幕偏移，不改几何。
双击标签会弹出数值编辑框，走 `editConstraintValue()` → `setDatum()` → `solve()`。

### 4.2 错误约束红色反馈

`constraintInError()` 检查 `lastConflicting/lastRedundant/lastMalformed` 等诊断
集合；错误约束的图标与标签会用红色绘制，为后续“点击高亮冲突约束”预留入口。

## 5. 参考源码

- `Moon/Sketcher/SketcherObjDraw.cpp`（本文主体）
- `Moon/Sketcher/SketcherObj.h`（`DrawOption`、`CurveSegment`）
- `Moon/Sketcher/SketcheTool2D.cpp`（`CurveConvert::toVector2D` 离散化）
- `Moon/Sketcher/SketcherObj.cpp`（`solve()` 后重建 mGeoSegment）
- 关联文档：[SketcherObj.md](SketcherObj.md)、[SketchConstraints.md](SketchConstraints.md)
