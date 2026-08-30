# 交互 Widget 设计实现架构（以 ClipPlane 剖切控件为例）

> 本文基于 `Moon/Interactive/EventWidget.*`、`Moon/Interactive/Im3DRenderer.*`、`Moon/Interactive/GizmoBehaviour.*`、`Moon/Interactive/Widgets/ClipPlane.cpp` 与 `Moon/Interactive/Im3DType.*` 的实际代码整理。

---

## 1. 概述

交互 Widget 是编辑器里的**可交互 3D 控件**：固定屏幕尺寸的 gizmo 网格 + 鼠标拾取 + 状态机驱动的拖拽/旋转。典型例子：

- `ClipPlane`：剖切平面控件（本文主线示例）——沿轴拖、在平面内拖、绕轴旋转，实时驱动 GPU 截面；
- `AxisTranslationWidget` / `ArrowRotateWidget` / `PadTaskWidget`：建模工具的平移/旋转手柄。

一个 Widget 由四层协作完成：

| 层 | 文件 | 职责 |
| --- | --- | --- |
| 事件层 | `EventWidget` + `RenderWindowInteractor` | 把 Qt 鼠标/键盘事件分发给 widget 的虚函数 |
| 绘制层 | `ImRenderer` | 立即模式 3D 绘制（网格/线/点）与 gizmo 拾取 |
| 几何层 | `GizmoBehaviour` | 射线与轴/平面/旋转平面的求交算法 |
| 业务层 | `ClipPlane` 等子类 | 状态机、控件外观、把交互结果写回场景/渲染器 |

![交互 Widget 整体架构与数据流](images/widget_architecture.svg)

---

## 2. 每帧驱动

编辑器每帧按以下顺序驱动：

```cpp
ImRenderer::newFrame(sceneView);   // 清空上一帧绘制列表、记录当前视图
// ... 业务层每帧渲染场景 ...
ImRenderer::endFrame();
//   └─ drawWidgets()：遍历 mGizmoWidgets，逐个调用 widget->update()
//   └─ drawMesh()：提交 drawMeshList（含 gizmo 网格）
```

`EventWidget::update()`：

```cpp
void EventWidget::update()
{
    mCurrentFrame = (mCurrentFrame + 1) % 1000000;   // 帧计数，用于鼠标事件节流
    if (mActive && mVisible)
    {
        onUpdate();                                   // 子类绘制控件 + 更新逻辑
    }
}
```

`EventWidget` 构造时把自己注册进 `ImRenderer::instance()`（`addGizmoWidget`），因此所有 widget 共享同一个绘制器与拾取缓冲。

---

## 3. 事件系统

### 3.1 Qt → RenderWindowInteractor

`RenderWindowInteractor::ReceiveEvent(QEvent*)` 接收 Qt 鼠标事件，转换成内部事件并广播：

```cpp
if (t == QEvent::MouseButtonPress || t == QEvent::MouseButtonRelease ||
    t == QEvent::MouseButtonDblClick || t == QEvent::MouseMove || t == QEvent::HoverMove)
{
    QMouseEvent* e2 = static_cast<QMouseEvent*>(e);
    SetEventInformationFlipY(e2->x(), e2->y(), ctrl, shift, 0, dblClick);
    if (t == QEvent::MouseMove || t == QEvent::HoverMove)
        InvokeEvent(ExecuteCommand::MouseMoveEvent, e2);
    else if (t == QEvent::MouseButtonPress)
        // switch(e2->button()) → InvokeEvent(LeftButtonPressEvent / ...)
}
```

`RenderWindowInteractor` 是**全局单例事件中心**：它维护一张观察者表（`EventObject::AddObserver(event, command, priority)`），`InvokeEvent` 按事件类型和优先级分发。

### 3.2 Interactor → EventWidget

`EventWidget` 构造时把自己挂到 Interactor 上，并通过 `CallbackMapper` 建立「事件 → 回调」映射：

| ExecuteCommand 事件 | WidgetEvent | 触发虚函数 |
| --- | --- | --- |
| `LeftButtonPressEvent` | `Select` | `onLeftMousePressed()` |
| `LeftButtonReleaseEvent` | `Select3D` | `onLeftMouseReleased()` |
| `RightButtonPressEvent` | `EndSelect` | `onRightMousePressed()` |
| `RightButtonReleaseEvent` | `Completed` | `onRightMouseReleased()` |
| `MouseMoveEvent` | `Move3D` | `onMouseMove()` |
| `KeyPressEvent` / `KeyReleaseEvent` | — | `onKeyPress()` / `onKeyRelease()` |

鼠标移动做了**帧节流**：静态入口 `EventWidget::MouseMove` 只在 `mCurrentFrame != mPreFrame` 时才调用 `onMouseMove()`，避免同一帧内重复处理。

### 3.3 直接订阅观察者

除了 CallbackMapper，widget 也可以直接向 Interactor 注册自己的回调。ClipPlane 用它来选择被剖切的模型：

```cpp
clickObserver = mSelf->Interactor->AddObserver(
    ExecuteCommand::LeftButtonReleaseEvent,
    this, &ClipPlane::ClipPlaneInternal::onMouseLeftClick, 0.0f);
```

`onMouseLeftClick` 拾取当前选中的 Actor，若带 `Model Renderer`，则用其世界包围盒初始化 widget 的位置与大小（`setupBox`）。

---

## 4. 绘制系统

### 4.1 两条绘制路径（灵活混用）

widget 的绘制并不局限于一种方式：**同一个 widget 里可以自由混用两条路径**，它们在本帧结束时分别提交。

| 路径 | 调用方式 | 数据去向 | 提交时机 |
| --- | --- | --- | --- |
| 网格路径 | `drawOneMesh(...)` / `drawOneFixScaleMesh(...)` | `drawMeshList`（预构建 `PolygonMesh`） | `endFrame → drawMesh()` |
| 立即模式路径 | `drawLine / drawPoint / drawTriangleList` | 顶点列表 `vertexData[0] / [1]`（按图层 + 图元类型） | `endFrame → drawSort() / drawUnsort()` |
| 2D / 覆盖层 | ImGui 等 | 独立 UI 上下文 | 独立提交 |

`endFrame()` 的提交顺序为 `runDrawTask → drawWidgets → drawMesh → drawSort → drawUnsort`：gizmo 网格先画，立即模式图元后画（排序 / 未排序两层，支持透明度与深度测试）。

以 ClipPlane 为例，同一个 `onUpdate()` 里就同时用到了三种方式：

- **网格路径**：`drawOneMesh(center, rotation, {0.1,0.1,0.1}, "TransformAxis")` 绘制三轴手柄 / 旋转环（固定屏幕尺寸）；
- **立即模式路径**：`drawLine` 画旋转圆弧、包围盒线框、圆心连线，`drawAlignedBox` 画盒线；
- **2D 覆盖**：`ImGui::GetForegroundDrawList()->AddText(...)` 显示旋转角度。

所以“有的部分是立即绘制、其它部分是 PolygonMesh”正是这套设计有意为之：需要固定缩放 / 深度 / 排序的实体手柄走 mesh 列表，临时路径与装饰线走立即模式，两者在 `onUpdate` 里按需混用。

### 4.2 立即模式 API

`ImRenderer` 提供类似立即模式 GUI 的 3D 绘制接口，widget 在 `onUpdate()` 里直接调用：

- `drawLine(a, b, size, color)` / `drawPoint(pos, size, color)` / `drawTriangleList(...)`：基础图元；
- `drawOneMesh(translation, rotation, scale, "TransformAxis")`：按名字绘制预构建的 gizmo 网格；
- `pushColor / popColor`、`pushSize / popSize`、`pushMatrix / popMatrix`：绘制状态栈；
- `getFrameParam()`：返回 `FrameParam`（eye、rayOrigin、rayDirection、cursor、viewport 尺寸、投影类型等），供几何计算使用。

立即模式调用按“图层 + 图元类型”写入本帧的顶点列表，`endFrame()` 里由 `drawSort()` / `drawUnsort()` 统一提交；网格调用进入 `drawMeshList`，由 `drawMesh()` 提交。所有列表在 `newFrame()` 清空。

### 4.3 Gizmo 网格的块结构

gizmo 网格（如 `TransformAxis()`，见 `Im3DType.cpp`）是一个 `PolygonMesh`，由若干**块（block）**组成：

```cpp
poly.addModel(cil, Identity, color);
poly.switchNextBlock({1,0,0,1}, "XAxis");   // 开启新块并命名
poly.addModel(cil, ...);
// ...
poly.addCell(cell);                          // 程序化多边形面
poly.switchNextBlock({0,1,0,1}, "YPlane");
```

- 每个块有一个 **blockId**（在块内顶点的 `w` 分量里），用于拾取解码；
- `getBlockId(name)` 按块名查 id，`setBlockColor(id, color)` 改颜色（悬停/激活高亮）；
- 整个网格有一个 **polygonId**（`setId`），`isSelectPolygon` 用它区分不同 gizmo。

### 4.4 固定屏幕尺寸（FIXED_SCALE）

gizmo 用 `GizmoCell.ovfx` 的 `FIXED_SCALE` 特性绘制，保证控件在任意相机距离下占用恒定像素数。原理与公式的详细推导见 [ClipPlane.cpp](../Moon/Interactive/Widgets/ClipPlane.cpp) 中 `ComputeGizmoFixedScaleRatio` 上方的注释：缩放后 1 个网格单位恒等于 200 像素。

这也是 ClipPlane 旋转圆弧必须用同一个 ratio 缩放的原因——否则控件固定大小、圆弧却随距离缩放，两者会脱节。

---

## 5. 拾取系统

拾取分为「写入」与「查询」两步。

### 5.1 写入：拾取 Pass 颜色编码

`PickingRenderPass`（渲染顺序 `Last`）每帧把 gizmo 网格画进**独立的拾取 framebuffer**（`actorPickingFramebuffer`）。核心是 `ImRenderer::drawMeshPick()`：以 `PICKING_PASS` 特性渲染 `drawMeshList`，片元输出编码：

```glsl
// GizmoCell.ovfx（PICKING_PASS）
fResult = vec4(0, int(round(fs_in.pos.w)) / 255.0, polygonId / 255.0, 254.0 / 255.0);
//           R=0      G=blockId（顶点 w）          B=polygonId           A=254（gizmo 标记）
```

该 Pass 只在“非相机操作、非正在拖拽”时启用（`SceneView::Update`），因此平时不产生额外开销。

### 5.2 查询：读回像素 → 解码 → 命中

`SceneView::HandleActorPicking` 读取光标处像素并解码：

```cpp
// PickingRenderPass::ReadbackPickingResult
if (pixel[3] == 254)   // gizmo 命中
{
    uint32_t polygonID = pixel[2];
    uint32_t blockID   = pixel[1];
    gizmoInstance.selectPolygon(polygonID, blockID);   // 写回 ImRenderer
    isSelected = true;
}
```

`selectPolygon(pid, bid)` 把当前命中结果存进 `ImRenderer` 的 `selectPolygonId / selectBlockId`。widget 随后在 `onMouseMove` 里逐块查询：

```cpp
bool hit = renderer->isSelectPolygon("TransformAxis", table[i].blockName);
// 等价于：TransformAxis().getId() == selectPolygonId && getBlockId(blockName) == selectBlockId
```

![Gizmo 拾取流程](images/widget_picking.svg)

---

## 6. ClipPlane 状态机（示例）

ClipPlane 用五个状态管理交互，手柄与操作的对应关系由一张表定义：

| 手柄块（blockName） | 交互类型 | 触发几何行为 |
| --- | --- | --- |
| `XArrow` / `YArrow` / `ZArrow` | 沿轴平移（`AxisT`） | `GizmoAxisTranslate` |
| `XPlane` / `YPlane` / `ZPlane` | 平面内平移（`PlaneT`） | `GizmoPlaneTranslate` |
| `XAxis` / `YAxis` / `ZAxis`（圆柱） | 绕轴旋转（`AxisR`） | `GizmoAxisRotate` |

![ClipPlane 交互状态机](images/widget_state_machine.svg)

### 6.1 状态转移

**Stop（初始）**

`onMouseMove` 遍历 9 个手柄块，`isSelectPolygon` 命中任一 → `mPickMesh = i`、`mState = Hot`、`setBlockColor(hotColor)` 高亮。

**Hot（悬停）**

- 再次移动：若不再命中任何块 → 回到 `Stop`，恢复原色；
- 左键按下：根据 `table[mPickMesh].meshId` 决定进入哪个拖拽状态，并初始化对应 gizmo 行为：

```cpp
// 沿轴拖：把轴与起点交给 transLatePick
m_internal->transLatePick.startPick(axis, m_internal->center);
mState = AxisT;

// 平面内拖：构造平面方程（法线 n，过 center）
float w = -m_internal->center.dot(normal);
m_internal->planeTPick.startPick({n.x, n.y, n.z, w}, m_internal->center);
mState = PlaneT;

// 绕轴旋转：绑定旋转轴 + 参考方向 + 参考点
m_internal->axisRPick.startPick(rotationAxis, m_internal->center, refDir, center + refDir);
mState = AxisR;
```

**AxisT / PlaneT / AxisR（拖拽中）**

`onMouseMove` 用 `getFrameParam()` 的射线原点/方向驱动 gizmo 行为，并把结果写回内部状态：

```cpp
// 平移：直接改 center；旋转：更新 xAxis/yAxis/zAxis 之一并正交化
m_internal->transLatePick.apply(rayDir, rayOrigin, m_internal->center);
m_internal->updateEngineUbo = true;    // 标记：下一帧同步平面到渲染器
```

旋转状态下 `onUpdate` 额外绘制角度文字、旋转路径圆弧（半径用 FIXED_SCALE ratio 缩放，见 §4.3）。

**松开（→ Hot）**

`onLeftMouseReleased` 恢复高亮色，并按 `updateFlag` 条件调用 `updateSection()` 刷新截面。

### 6.2 与截面渲染的联动

ClipPlane 只负责“交互”，不直接生成几何。拖拽时置 `updateEngineUbo = true`，`onUpdate` 中把平面写入渲染器：

```cpp
auto& feature = m_sceneView->GetRenderer().GetFeature<EngineBufferRenderFeature>();
feature.SetClipPlane(zAxis.x, zAxis.y, zAxis.z, -zAxis.dot(center));
```

GPU 侧（`SectionCapRenderPass` / `SectionContourRenderPass`）每帧读 `ubo_plane` 生成截面与截线——交互与渲染解耦，这也是“拖动即实时更新”的原因。原理见 [SectionRendering.md](SectionRendering.md)。

---

## 7. 几何交互算法（GizmoBehaviour）

### 7.1 GizmoAxisTranslate（沿轴拖）

把鼠标射线投影到轴上，取射线上离轴最近的点，维护按下时的初始偏移：

```cpp
// 射线 P(t) = eye + t * ray，轴 center + s * axis
// 求 min|P(t) - (center + s*axis)| 得到 t，再投影出轴上点
```

### 7.2 GizmoPlaneTranslate（平面内拖）

射线与平面求交：

```cpp
float t = -(eye.dot(n) + d) / ray.dot(n);   // n 为平面法线，d 为平面常数
pos = eye + ray * t;
```

### 7.3 GizmoAxisRotate（绕轴旋转）

1. **平面投影**：把射线与过中心的旋转平面求交，交点相对中心的向量投影到平面并归一化（`computePlaneProj`）；
2. **增量角**：相邻两帧投影方向夹角，用 `atan2(axis·(a×b), a·b)` 带符号累计（`computeAngle`）；
3. **累积角 + 吸附**：`m_totalAngle` 累加后按 1° 吸附（`enableSnap`）；
4. **输出**：`Eigen::AngleAxisf(snapAngle, axis)` 旋转参考方向/参考位置；
5. **圆弧**：`getRotationArc()` 在参考方向与当前方向之间插值出一圈弧点，供 `onUpdate` 绘制旋转路径。

---

## 8. 如何扩展一个新 Widget

1. **继承 `EventWidget`**，构造时传入名字（自动注册到 `ImRenderer` 与 `RenderWindowInteractor`）；
2. **重写 `onUpdate()`**：用 `renderer->drawOneMesh(...)` / `drawLine(...)` 绘制控件，读取 `getFrameParam()` 做逻辑；
3. **在 `Im3DType` 里构建或复用 gizmo 网格**：`switchNextBlock` 分块、命名，保证每个手柄可拾取；
4. **重写 `onMouseMove()`**：用 `isSelectPolygon(pname, blockName)` 做悬停检测，切换状态机；
5. **重写 `onLeftMousePressed()` / `onLeftMouseReleased()`**：进入/退出拖拽，初始化 `GizmoBehaviour`；
6. **把交互结果写回场景**（如 `SetClipPlane`、`InvokeEvent(自定义事件)` 通知业务层）；
7. 需要接收键盘时重写 `onKeyPress` / `onKeyRelease`。

---

## 9. 关键文件

| 文件 | 职责 |
| --- | --- |
| `Moon/Interactive/EventWidget.*` | widget 基类：事件回调映射、生命周期、帧节流 |
| `Moon/Interactive/Interactive/RenderWindowInteractor.*` | Qt 事件 → 内部事件分发中心 |
| `Moon/Interactive/Im3DRenderer.*` | 立即模式绘制、gizmo 网格提交、拾取查询 |
| `Moon/Interactive/Im3DType.*` | `PolygonMesh` 块结构、`TransformAxis()` 等 gizmo 构建 |
| `Moon/Interactive/GizmoBehaviour.*` | 轴/平面/旋转几何算法 |
| `Moon/Interactive/Widgets/ClipPlane.*` | 剖切控件：状态机、手柄表、截面联动（示例） |
| `Moon/renderer/PickingRenderPass.cpp` | 拾取 framebuffer、像素读回与 `selectPolygon` |
| `Moon/Interactive/Im3DRenderer.h` | `FrameParam`（射线/光标/视口信息） |
