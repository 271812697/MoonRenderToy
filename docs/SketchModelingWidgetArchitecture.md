# 草图建模交互 Widget 架构设计（DrawSketchHandler 家族）

> 本文基于 `Moon/Interactive/EventWidget.*`、`Moon/Interactive/Widgets/DrawSketchHandler.*`、
> `Moon/Interactive/Widgets/DrawSketchDefaultHandler.h`、`Moon/Interactive/Widgets/DrawSketchHandlerArc.*`、
> `Moon/Sketcher/SketcherObj.*`、`Moon/editor/Toolbar/sketchToolbar.cpp` 与
> `Moon/renderer/GizmoRenderPass.cpp` 实际代码整理。
>
> 本文是**架构总览**；逐行级的机制细节（事件→交互、预览管线、状态转移、提交时序）见
> [SketchModelingWidget.md](./SketchModelingWidget.md)。

---

## 1. 设计目标与核心思想

草图建模工具要解决三个正交的问题：

1. **事件**：Qt 的鼠标/键盘事件如何到达“当前激活的绘图工具”；
2. **坐标**：屏幕上的光标如何换算成草图平面上的 2D 坐标（含吸附）；
3. **状态**：一次“画圆弧”需要三次点击，三次点击分别代表什么、数据如何累积、何时提交。

架构的核心决策是**把这三个问题分别交给三个独立的机制**，再通过一个模板基类把它们粘起来：

| 问题 | 机制 | 关键类 |
| --- | --- | --- |
| 事件 | 观察者 + 回调映射 | `RenderWindowInteractor` → `EventWidget` |
| 坐标 | 射线-平面求交 + 平面基投影 + 吸附 | `DrawSketchHandler` |
| 状态 | 泛型状态机 + 构造方式机 + 模板方法 | `StateMachine<T>` / `ConstructionMethodMachine<T>` / `DrawSketchDefaultHandler<…>` |

这实际上是把 FreeCAD SketcherGui 的交互模型**映射**到自研 widget 体系：

| FreeCAD 概念 | Moon 实现 |
| --- | --- |
| `DrawSketchHandler`（草图领域基类） | `DrawSketchHandler`（投影/吸附/预览辅助） |
| `DrawSketchHandler::MouseMove` 等 | `EventWidget` 静态入口 + 帧节流 |
| `StateMachine / ConstructionMethodMachine` | 同名泛型模板 |
| `DrawSketchHandlerArc` 的三点/圆心画弧 | `DrawSketchHandlerArc`（逻辑同构） |
| 约束/自动约束 | 已剥离，保留 `snapPoint` 简单吸附 |

---

## 2. 分层结构与依赖方向

![类层次结构](images/sketch_class_hierarchy.svg)

依赖方向**自上而下、只允许向下依赖**：

```
EventWidget                        （自研交互基类，与草图无关）
   ▲
DrawSketchHandler                  （草图领域：平面/坐标/吸附/预览）
   ▲                    ▲                ▲
DrawSketchDefaultHandler     StateMachine<T>   ConstructionMethodMachine<T>
   ▲                        （继承/组合 mixin）
DrawSketchHandlerArc 等具体工具
```

`DrawSketchHandler` 不知道“画的是圆弧还是直线”；`DrawSketchDefaultHandler` 不知道“一次画几个点、
每个点代表什么”；只有最底层的具体工具（如 `DrawSketchHandlerArc`）才持有圆弧语义（圆心、半径、
起止角）。**越上层越通用，越下层越具体**，这是整个设计可扩展的关键。

### 2.1 各层契约

| 类 | 提供的接口（契约） |
| --- | --- |
| `EventWidget` | `setActive/setVisible/update`；虚函数 `onUpdate/onMouseMove/onLeftMousePressed/…/onKeyPress`（默认空实现） |
| `DrawSketchHandler` | `makePlane(plane)`、`onSketchPos`（每帧刷新）、`drawEdit(...)`、`drawPositionAtCursor/drawFloatValue`、`getWorldPosFromSketchPos` |
| `StateMachine<T>` | `setState/ensureState/moveToNextMode/reset/isFirstState/isLastState/setNextState/applyNextState` |
| `ConstructionMethodMachine<T>` | `iterateToNextConstructionMethod/constructionMethod()` |
| `DrawSketchDefaultHandler<…>` | 框架流程：`onButtonPressed → updateDataAndDrawToPosition → canGoToNextMode → moveToNextMode`；`finish → executeCommands`；几何工厂 `add*ToShapeGeometry` |
| `DrawSketchHandlerArc` | 覆写 `updateDataAndDrawToPosition / createShape / canGoToNextMode` |

---

## 3. 事件层：观察者 + 回调映射

### 3.1 事件链

```
Qt QMouseEvent
  → RenderWindowInteractor::ReceiveEvent（全局单例，按事件类型 InvokeEvent）
    → 观察者表里注册的 EventWidget 回调（按优先级分发）
      → 静态入口 LeftMousePressed / MouseMove / …
        → 虚函数 onLeftMousePressed() / onMouseMove()
```

`EventWidget` 构造时完成三件事：

```cpp
ImRenderer::instance().addGizmoWidget(this);          // 1. 注册进绘制器（每帧驱动）
SetInteractor(RenderWindowInteractor::Instance());    // 2. 挂到全局事件中心
m_sceneView = &GetService(Editor::Panels::SceneView); // 3. 拿到场景视图（射线/坐标）

// 4. 用 CallbackMapper 建立「事件 → 回调」映射表
CallbackMapper->SetCallbackMethod(ExecuteCommand::LeftButtonPressEvent, GizmoEvent::AnyModifier, 0,0,0,
                                  WidgetEvent::Select, this, EventWidget::LeftMousePressed);
CallbackMapper->SetCallbackMethod(ExecuteCommand::MouseMoveEvent,      GizmoEvent::AnyModifier, 0,0,0,
                                  WidgetEvent::Move3D,  this, EventWidget::MouseMove);
// … 其余：LeftButtonRelease→Select3D、RightButtonPress→EndSelect、RightButtonRelease→Completed
```

映射表：

| ExecuteCommand 事件 | WidgetEvent | 静态入口 | 虚函数 |
| --- | --- | --- | --- |
| `LeftButtonPressEvent` | `Select` | `LeftMousePressed` | `onLeftMousePressed()` |
| `LeftButtonReleaseEvent` | `Select3D` | `LeftMouseReleased` | `onLeftMouseReleased()` |
| `RightButtonPressEvent` | `EndSelect` | `RightMousePressed` | `onRightMousePressed()` |
| `RightButtonReleaseEvent` | `Completed` | `RightMouseReleased` | `onRightMouseReleased()` |
| `MouseMoveEvent` | `Move3D` | `MouseMove` | `onMouseMove()` |
| `KeyPress/KeyReleaseEvent` | — | `ProcessKeyEvents`（观察者） | `onKeyPress/onKeyRelease(key)` |

### 3.2 两个关键设计

**a) 静态入口 + 帧节流**

观察者回调需要 C 函数指针，所以基类提供静态函数，内部转回对象再分发：

```cpp
void EventWidget::MouseMove(AbstractWidget* w) {
    EventWidget* self = reinterpret_cast<EventWidget*>(w);
    if (self->mCurrentFrame != self->mPreFrame) {   // 同一帧内只处理一次
        self->mPreFrame = self->mCurrentFrame;
        self->onMouseMove();
    }
}
```

`update()` 每帧先递增 `mCurrentFrame`，配合 `mPreFrame` 实现移动事件的帧节流，避免一帧内多次
鼠标移动造成重复计算。

**b) `setActive` 的副作用**

```cpp
void EventWidget::setActive(bool flag) {
    mActive = flag;
    onSetActive(flag);          // 子类可感知激活（绘制样式等）
    SetEnabled(flag ? 1 : 0);   // 关键：决定键盘观察者是否注册
}
```

`SetEnabled` 只做一件事：激活时向 Interactor 注册 `KeyPressEvent/KeyReleaseEvent` 观察者，停用时
移除。也就是说**键盘事件只在工具激活时可达**，而鼠标事件始终可达（由 `mActive` 在 `update()` 里
拦绘制、由子类逻辑拦响应）。

---

## 4. 坐标层：射线 → 平面 → 2D → 吸附

`DrawSketchHandler::onMouseMove()` 每帧把光标换算成草图坐标：

```cpp
auto ray = m_sceneView->GetMouseRay();
ray.hitPlane(FVector3(plane.normal.x, plane.normal.y, plane.normal.z),
             plane.normal.Dot(plane.origin), out);          // ① 射线与平面求交（世界坐标）

Base::Vector3d hitPos(out.x, out.y, out.z);
double x = (hitPos - plane.origin).Dot(plane.xAxis);        // ② 投影到平面基 → 2D
double y = (hitPos - plane.origin).Dot(plane.yAxis);
onSketchPos = Base::Vector2d(int(x*100)/100.0, int(y*100)/100.0);  // ③ 量化到 0.01
isSnapedSketchPos = sketchobj ? sketchobj->snapPoint(onSketchPos) : false;  // ④ 吸附
```

对应四步数学：

1. **求交**：草图平面用隐式方程 `n·p = d` 描述（`d = n·origin`），射线与平面求交得到世界点；
2. **投影**：把世界点相对 `origin` 的向量分别点乘 `xAxis`/`yAxis`，得到平面上的 2D 坐标；
3. **量化**：取两位小数，保证连续帧间坐标稳定、避免浮点抖动；
4. **吸附**：`SketcherObj::snapPoint` 把坐标吸到已有几何的特征点上。

吸附实现要点（`SketcherObj::snapPoint`）：

```cpp
// 把 2D 点变换到屏幕空间，逐特征点算屏幕距离，阈值 10px，取最近者
Base::Matrix4D trans = pla * getplaneTransform();   // 平面 → 视口矩阵
for (每个几何的每个离散点 sep) {
    double dist = (screenPos - trans * sep.coord).Length();
    if (dist < 10.0 && dist < minDist) { minDist = dist; pos = sep.coord; ret = true; }
}
// 未命中时兜底：原点 / X 轴 / Y 轴
```

> 吸附判定在**屏幕空间**而非草图空间，因此 10px 是“所见即所得”的鼠标容差，与相机缩放无关。

---

## 5. 状态层：泛型状态机 + 模板方法

### 5.1 状态机模板 `StateMachine<T>`

状态用枚举表达，`End` 必须是最后一个成员（哨兵）：

```cpp
enum class ThreeSeekEnd { SeekFirst, SeekSecond, SeekThird, End };
```

模板内部把枚举转成整数推进：

```cpp
SelectModeT computeNextMode() const {
    auto modeint = static_cast<int>(state());
    return modeint < maxMode ? static_cast<SelectModeT>(modeint + 1) : SelectModeT::End;
}
void moveToNextMode() { setState(computeNextMode()); }
```

`maxMode = static_cast<int>(T::End)` 即“状态总数”，所以 `End` 必须最后、状态数量由枚举自动决定。
`setNextState/applyNextState` 支持延迟切换（先记录、稍后应用），`ensureStateIfEarlier` 允许在需要
回退修改时回到更早的已完成状态。

### 5.2 构造方式机 `ConstructionMethodMachine<T>`

与状态机正交的第二个轴：同一个工具可以有多种“画法”（圆弧的**圆心式** vs **三点式**），`M` 键在
`onKeyPress` 里调用 `iterateToNextConstructionMethod()` 循环切换。

### 5.3 模板方法：框架把流程写死，子类只填钩子

`DrawSketchDefaultHandler<HandlerT, SelectModeT, N, ConstructionMethodT>` 是核心设计：

```cpp
// 左键按下（框架固定流程）
virtual void onLeftMousePressed() override { ButtonPressParse(); }
void ButtonPressParse() { onButtonPressed(onSketchPos); }

virtual void onButtonPressed(Base::Vector2d pos) {
    this->updateDataAndDrawToPosition(pos);   // ← 钩子 1：子类更新中间数据
    if (canGoToNextMode()) {                  // ← 钩子 2：子类校验
        this->moveToNextMode();               // 框架推进状态
    }
}
```

左键释放 → `releaseButton` → `finish()`：状态为 `End` 时执行 `executeCommands()`，把
`ShapeGeometry` 加入 `SketcherObj`。

![模板方法：一次左键点击的框架调用链](images/sketch_handler_template_flow.svg)

**框架固定**的部分（子类不可改、也不该改）：

| 流程 | 固定逻辑 |
| --- | --- |
| `onLeftMousePressed` → `onButtonPressed` | 先刷新预览再推进状态，顺序不可颠倒 |
| `finish()` | 只有 `End` 态才提交 |
| `handleContinuousMode()` | 连续模式下 `reset()` 回 `SeekFirst` |
| `executeCommands()` | 遍历 `ShapeGeometry` 逐个 `addGeometry` |
| `CreateAndDrawShapeGeometry()` | `clearEdit → createShape → drawEdit` 三段式刷新预览 |
| `rightButtonOrEsc()` | 第一步 `quit()`；后续步骤 `handleContinuousMode()` |

**子类只需覆写**三个钩子：

| 钩子 | 职责 | Arc 的实现 |
| --- | --- | --- |
| `updateDataAndDrawToPosition(pos)` | 用当前光标更新中间数据 | 按 state 分支更新 `centerPoint/firstPoint/secondPoint/radius/startAngle/endAngle/arcAngle` |
| `canGoToNextMode()` | 校验本次点击是否有效 | 半径/弧角非零 |
| `createShape(bool)` | 把中间数据转成预览几何 | SeekSecond 画整圆辅助、SeekThird 生成圆弧 |

这就是典型的**模板方法模式**：算法骨架在基类，可变步骤延迟到子类。`HandlerT` 模板参数（CRTP
风格）让基类无需虚表也能以具体类型复用（如 `DrawSketchDefaultHandler<DrawSketchHandlerArc, …>`）。

### 5.4 几何所有权

`ShapeGeometry` 是 `std::vector<std::unique_ptr<Part::Geometry>>`；工厂函数返回裸指针便于立即
操作，所有权始终在容器：

```cpp
auto addArcToShapeGeometry(Base::Vector3d p1, double start, double end, double radius, bool constructionMode) {
    auto arc = std::make_unique<Part::GeomArcOfCircle>();
    arc->setCenter(p1); arc->setRange(start, end, true); arc->setRadius(radius);
    return static_cast<Part::GeomArcOfCircle*>(ShapeGeometry.emplace_back(std::move(arc)).get());
}
```

提交时 `SketcherObj::addGeometry(Part::Geometry*)` 内部会 `copy()` **深拷贝**一份：

```cpp
int SketcherObj::addGeometry(Part::Geometry* curve) {
    std::unique_ptr<Part::Geometry> temp(curve->copy());   // 深拷贝，与 handler 解耦
    return addGeometry(temp);                              // 同时缓存离散段 mGeoSegment
}
```

因此 handler 在 `reset()` 里清空 `ShapeGeometry` 不会影响已提交到草图的几何。

---

## 6. 圆弧工具的领域逻辑

`DrawSketchHandlerArc` 用 Pimpl（`m_internal`）持有中间状态：

```cpp
Base::Vector2d centerPoint, firstPoint, secondPoint;
double radius, startAngle, endAngle, arcAngle;
```

### 6.1 Center（圆心式）

```cpp
// SeekSecond：点2 定半径与起始角
firstPoint = onSketchPos;
startAngle = (firstPoint - centerPoint).Angle();
radius     = (onSketchPos - centerPoint).Length();

// SeekThird：点3 定扫掠角（与上一帧 arcAngle 做连续性选择顺/逆时针）
angle1 = (pos - centerPoint).Angle() - startAngle;
angle2 = angle1 + (angle1 < 0 ? 2 : -2) * PI;
arcAngle = |angle1 - old| < |angle2 - old| ? angle1 : angle2;
// arcAngle 符号决定 start/end 谁先谁后
```

### 6.2 ThreeRim（三点式）

```cpp
// SeekSecond：先用两端点中点当临时圆心
centerPoint = (onSketchPos - firstPoint) / 2 + firstPoint;
// SeekThird：三点求外接圆圆心
centerPoint = getCircleCenter(firstPoint, secondPoint, onSketchPos);
radius = (onSketchPos - centerPoint).Length();
// 恒逆时针：α3 是否落在 α1、α2 之间决定小弧还是补角大弧
if (angle3 > min(α1,α2) && angle3 < max(α1,α2))
    { start = min; end = max; arcAngle = end - start; }        // 小弧
else
    { start = max; end = min; arcAngle = 2π - (start - end); } // 补角大弧
```

![ThreeRim 三点画弧几何原理](images/sketch_arc_geometry.svg)

`getCircleCenter` 用三点外接圆公式：

```
u = P2−P1, v = P3−P2, w = P1−P3
w0 = 2·sqrt(|uu·ww − uw²|)·uw/(uu·ww)   （对 w1、w2 同理）
C  = (w0·P1 + w1·P2 + w2·P3) / (w0 + w1 + w2)
```

（重合/共线的异常抛出被注释，由 `canGoToNextMode` 的半径/弧角守卫兜底。）

### 6.3 预览生成

```cpp
void DrawSketchHandlerArc::createShape(bool) {
    ShapeGeometry.clear();
    if (radius < Precision::Confusion()) return;
    if (state() == SeekSecond)
        addCircleToShapeGeometry(center, radius, true);   // 整圆辅助
    else if (fabs(arcAngle) > Precision::Confusion())
        addArcToShapeGeometry(center, startAngle, endAngle, radius, true);
}
```

`CreateAndDrawShapeGeometry()` → `clearEdit() + createShape(true) + drawEdit(ShapeGeometry)`，
`drawEdit` 把几何用 `CurveConvert::toVector2D`（50 段）离散成成对顶点写入 `lines`，`onUpdate`
每帧画黄色折线。**预览是“数据 → 离散折线”的立即模式绘制**，随光标实时重建。

---

## 7. 数据流全景与生命周期

![事件与坐标数据流](images/sketch_event_flow.svg)

### 7.1 一次完整的“画圆弧”

1. 工具栏点 Arc → `CreateCurveCommand::execute` → `enableGizmoWidget("DrawSketchHandlerArc", true)`
   → `setActive(true)`，blackList 取消其它工具；
2. 鼠标移动 → `onMouseMove` → 射线/平面求交 → `onSketchPos`（两位小数 + 吸附）→ 状态机各阶段刷新预览；
3. 三次左键：`onButtonPressed` → 各状态捕获数据 → 校验 → 推进状态 → `SeekThird` 阶段生成完整圆弧；
4. 第三次左键释放 → `finish()`（状态 `End`）→ `executeCommands()` → `SketcherObj::addGeometry`
   （深拷贝 + 离散段缓存）；
5. 连续模式 → `reset()` 回 `SeekFirst` 继续画下一个；右键/Esc 第一步 → `quit()`
   （`setActive(false)` + 取消工具栏勾选）。

### 7.2 注册与销毁

所有 handler 由 `GizmoRenderPass` 在构造时 `new` 出来，存入 `mWidgets` map，pass 析构时统一
`delete`；`EventWidget` 析构时自动 `removeGizmoWidget` 并移除事件观察者。工具本身**不随激活
创建/销毁**，而是常驻、仅切换 `mActive`——避免高频开关时反复 new/delete 与重建观察者。

---

## 8. 设计模式总结

| 模式 | 体现在哪 | 解决的问题 |
| --- | --- | --- |
| 模板方法 | `DrawSketchDefaultHandler` 固定调用链，三个虚钩子给子类 | 复用“点击→推进→提交”骨架，子类只写领域逻辑 |
| 状态机 | `StateMachine<T>`（枚举 + End 哨兵） | 多点击工具的阶段性数据累积 |
| 策略/构造方式 | `ConstructionMethodMachine<T>` + `M` 键 | 同一工具多种画法 |
| CRTP/静态多态 | `HandlerT` 模板参数 | 基类复用具体类型，避免虚表开销与强转 |
| 观察者 | `RenderWindowInteractor` + CallbackMapper | 事件解耦、多 widget 共存 |
| 服务定位器 | `GetService(SceneView / SketchToolbar / …)` | 跨模块解耦 |
| 工厂 | `GizmoRenderPass` 统一创建注册；`add*ToShapeGeometry` 工厂 | 工具注册与几何构造 |
| Pimpl | `m_internal` | 隐藏圆弧中间状态、稳定 ABI |
| 立即模式渲染 | `Im3DRenderer` + `lines` 缓冲 | 预览随光标实时重建，无需持久场景对象 |

---

## 9. 扩展新工具（为什么只需要四步）

由于骨架和状态机都泛型化了，新增工具：

1. 定义状态枚举（`End` 必须最后）；
2. 继承 `DrawSketchDefaultHandler<X, 你的状态机, N, 你的构造方式>`；
3. 实现 `updateDataAndDrawToPosition / createShape / canGoToNextMode`；
4. `GizmoRenderPass` 注册 + `SketchToolbar` 加 `CreateCurveCommand` 并入 `blackList`。

```cpp
class DrawSketchHandlerX : public DrawSketchDefaultHandler<DrawSketchHandlerX, StateMachines::TwoSeekEnd, 2,
                                                          ConstructionMethods::DefaultConstructionMethod> {
public:
    DrawSketchHandlerX(const std::string& name)
        : DrawSketchDefaultHandler<DrawSketchHandlerX, StateMachines::TwoSeekEnd, 2,
                                   ConstructionMethods::DefaultConstructionMethod>(name) {}
    void updateDataAndDrawToPosition(Base::Vector2d pos) override { /* … */ }
    bool canGoToNextMode() override { return true; }
    void createShape(bool) override { /* addLineToShapeGeometry(…) */ }
};
```

不需要碰事件层、坐标层、渲染层，也不需要碰其它工具——这就是第 2 节“依赖方向自上而下”带来的
扩展收益。

---

## 10. 已知注意点

- `executeCommands()` 日志字符串末尾带空格（`"add {0} geometry to sketcher obj "`），会污染日志面板；
- `PInitAutoConstraintSize` 模板参数目前未使用（FreeCAD 的自动约束被剥离后的残留）；
- `constructionMode` 参数未接入 `GeometryFacade::setConstruction`，暂无构造线/参考线区分；
- `getCircleCenter / areCollinear` 的异常抛出被注释，极端输入靠 `canGoToNextMode` 兜底；
- 角度常量建议统一 `M_PI` 或 `std::numbers::pi`。
