# DrawSketchHandlerBSpline：样条工具

## 1. 概述

两个最复杂的模板工具之一（另一个是 Rectangle）。支持两种构造方式：

- **ControlPoints（控制点式）**：点击放置控制多边形顶点，生成通过插值/逼近的 B 样条
  （degree 3，端点重复度 4）；
- **Knots（插值式）**：点击放置型值点，同时给每点生成切向量手柄，用 OCCT `interpolate`
  插值生成样条。

```cpp
class DrawSketchHandlerBSpline : public DrawSketchDefaultHandler<DrawSketchHandlerBSpline,
                                                                 StateMachines::ThreeSeekEnd, 2,
                                                                 BSplineConstructionMethod>
```

## 2. 关键成员

```cpp
std::vector<Base::Vector2d> points;      // 控制点 / 型值点
std::vector<Base::Vector2d> tangents;    // Knots 模式每点的切向量
int hotPointId, hotTangentId;            // 悬停命中的点/切柄
bool isPointActive, isTangentActive;     // 是否正在拖拽
bool periodic;                           // P 键切换闭合
size_t SplineDegree = 3;
```

## 3. 与模板默认流程不同的地方

### 3.1 状态机：停留在 SeekSecond 积累点

`canGoToNextMode`：

```cpp
if (state() == SeekFirst) {
    if (!addPos()) return false;          // 第一个点
}
else if (state() == SeekSecond) {
    if (与最后一点重合) return false;      // 防重
    if (!addPos()) return false;          // 继续加点
    return isClosed;                      // 当前恒 false（闭合判定代码被注释）
}
```

`isClosed` 的判定依赖 FreeCAD 的共点约束，被注释后恒为 `false`——意味着**点击永远不会把状态
从 SeekSecond 推到 End**。样条的收尾实际走 `quit()`：

```cpp
void quit() {
    if (state() == SeekSecond) {
        if (geoIds.size() > 1) { setState(End); finish(); }  // 右键收尾：提交
        else { handleContinuousMode(); }                      // 点太少：直接重置
    } else {
        DrawSketchHandler::quit();
    }
}
```

所以 BSpline 是“**点击加点、右键收尾**”的工具，与圆弧“点击到 End”不同。

### 3.2 热点拖拽：点/切柄可编辑

`onMouseMove` 里做了三件事：

1. 正在拖点（`isPointActive`）→ `points[hotPointId] = onSketchPos`；
2. 正在拖切柄（`isTangentActive`，Knots 模式）→ 按正/反方向更新 `tangents[hotTangentId]`；
3. 否则做悬停命中：在 `tolerance=2.0` 内命中控制点 → `hotPointId`；Knots 模式下命中切柄两端 →
   `hotTangentId` + `isForwardTangent`。

`onUpdate` 按状态给点/切柄上色：普通白、悬停洋红、激活红。

### 3.3 快捷键

```cpp
onKeyPress:  "P" → periodic = !periodic    // 开/闭
             "B" → 移除最后一个点（同步 pop 各 vector）
             "M" → 交给基类切换构造方式
```

## 4. 几何生成

```cpp
// ControlPoints：构造控制多边形样条
auto bSpline = std::make_unique<Part::GeomBSplineCurve>(poles, weights, knots, mults, degree, periodic);
bSpline->setPoles(bsplinePoints3D);

// Knots：插值
auto bSpline = std::make_unique<Part::GeomBSplineCurve>();
bSpline->interpolate(points3D, tangents3D, periodic);   // 失败被 catch 静默
```

## 5. 注意点

- `onLeftMouseReleased` 里调用的是 `SuperClass::onRightMouseReleased()`（应为
  `onLeftMouseReleased`）——疑似复制粘贴错误，左键释放时状态标记复位逻辑受影响；
- `geoIds` 推入的字面量 1 是占位（FreeCAD 约束代码被注释后的残留），`addGeometry` 本体是空
  桩直接返回 true；
- `PInitAutoConstraintSize` 传的是 2（模板第 3 参数），与圆弧的 3 不同，但同样未使用。
