# DrawSketchHandlerOffset：偏移工具

## 1. 概述

对**当前选中的草图几何**做等距偏移：支持直线、圆、圆弧，以及首尾相连的
折线/弧链（矩形、多边形等闭合轮廓）。偏移计算交给 OCC 的
`BRepOffsetAPI_MakeOffset`，结果转回 `Part::Geometry` 后走常规提交路径加入草图。

```cpp
class DrawSketchHandlerOffset
    : public DrawSketchDefaultHandler<DrawSketchHandlerOffset,
                                      StateMachines::OneSeekEnd, 0,
                                      ConstructionMethods::OffsetConstructionMethod>
```

它是一个**一次性（非连续）工具**：`continuousMode = false`，单击确定偏移距离后
立即提交并 `quit()`，不会像直线/圆那样留在工具里继续画下一条。

## 2. 交互流程

工具不对“点击处”做几何拾取，它偏移的是**进入工具前已经选中的几何**：

```
[选中曲线] → 点工具栏 Offset（激活并重置） → SeekFirst（移动鼠标实时预览）
    → 左键按下：记录 endpoint、更新预览 → 状态机推进到 End
    → 左键释放：executeCommands 提交 → quit()（工具退出、按钮弹起）
    → 右键（仍处 SeekFirst）：直接取消退出
```

选择来源是 `SketcherObj::getSelectIds()`，`onUpdate()` 每帧刷新 `listOfGeoIds`。
默认单选是覆盖式（`OverrideSelect`），框选/按 Ctrl 可多选。

### 键盘

- `M`：切换构造方式 `OffsetConstructionMethod`（Arc=圆角连接 / Intersection=尖角连接），
  对应 `mkOffset.Init` 的 join type 0/2；
- `D`：切换是否删除原始几何（`deleteOriginal`），默认保留。

## 3. 内部数据

```cpp
std::vector<int> listOfGeoIds;        // refreshed from getSelectIds() every frame
std::vector<TopoDS_Wire> sourceWires; // selected edges chained into wires
Base::Vector2d endpoint;              // cursor position on the sketch plane
bool onlySingleLines;                 // true when every chain is a single edge
bool deleteOriginal;                  // toggled by D
bool offsetLengthSet;                 // distance already solved
double offsetLength;                  // offset distance (negative = inward for closed wires)
// ShapeGeometry (base member) holds the preview / geometries to commit
```

## 4. 源线框组装：generateSourceWires

1. 对每个选中的几何：深拷贝 → `reverseIfReversed()` → `toShape()` 取 `TopoDS_Edge`，
   并读取两端点；
2. **贪心连链**：以 1e-6 容差按共享端点把边拼成有序链（缺边自动反向再接）；
3. `BRepBuilderAPI_MakeWire` 合成 wire，失败则退化为逐条边单独偏移；
4. 闭合 wire 强制按草图平面 +Z 为 **CCW**，保证“正距离=外扩、负距离=内缩”
   与 FreeCAD 的符号约定一致；
5. `onlySingleLines`：所有链都只有单条边时为 true（影响偏移构造器是否带平面）。

## 5. 偏移距离：findOffsetLength

用点击点在平面上的投影构造 `TopoDS_Vertex`，对每条 wire 做
`BRepExtrema_DistShapeShape` 取**最近距离**（多选时取最小值）；若点在闭合 wire
内部（`BRepClass_FaceClassifier` 判定 `TopAbs_IN`），距离取负——即向里偏移。

## 6. 偏移计算：buildOffsetGeometry

```cpp
// Arc = 0, Intersection = 2 (switched with M)
const short joinType = constructionMethod()
    == ConstructionMethods::OffsetConstructionMethod::Intersection ? 2 : 0;

BRepOffsetAPI_MakeOffset mkOffset;
if (onlySingleLines) {
    // a single line has no intrinsic offset side; provide a working plane
    TopoDS_Face plane = BRepBuilderAPI_MakeFace(
        gp_Pln(gp_Pnt(0.0, 0.0, 0.0), gp_Dir(0.0, 0.0, 1.0)));
    mkOffset = BRepOffsetAPI_MakeOffset(plane);
}
mkOffset.Init(GeomAbs_JoinType(joinType), false);
for (auto& wire : sourceWires) mkOffset.AddWire(wire);
mkOffset.Perform(offsetLength);   // Standard_Failure -> CORE_ERROR and abort
```

成功后用 `TopExp_Explorer` 遍历结果边并转回几何。

## 7. 结果边转几何：CurveToPartGeometry

- 直线：两端点 → `GeomLineSegment`；
- 整圆：`beg == end` → `GeomCircle`（中心/半径）；
- 圆弧：`Handle(Geom_TrimmedCurve) tCurve = new Geom_TrimmedCurve(hCircle, u1, u2)`
  → `GeomArcOfCircle::setHandle(tCurve)` → `reverseIfReversed()`；
  这是 FreeCAD 的正规做法，修复了此前矩形外偏时 3 个角弧错位的问题；
- 其它类型（椭圆/样条）：暂不支持，返回 `nullptr`，该偏移边会被静默丢弃。

## 8. 预览与提交

`updateDataAndDrawToPosition` 只在 `SeekFirst` 生效：更新 `endpoint` →
`findOffsetLength` → `buildOffsetGeometry` → `CreateAndDrawShapeGeometry`，
因此**移动鼠标即可实时预览偏移结果**。

```cpp
void executeCommands() override {
    if (sourceWires.empty())   generateSourceWires();
    if (!offsetLengthSet)      findOffsetLength();
    if (std::fabs(offsetLength) > Precision::Confusion()) {
        buildOffsetGeometry();
        SupperClass::executeCommands();   // add each ShapeGeometry entry to the sketch
        if (deleteOriginal) deleteOriginalGeos();
    }
    quit();   // one-shot tool: exit whether the offset succeeded or not
}
```

## 9. 为什么必须“激活时重置”和“提交后强制退出”

工具栏里的 widget 是 **GizmoRenderPass 中常驻的单例**，点按钮只做
`setActive(true/false)`，对象本身不会销毁重建。因此有两个关键设计：

1. **一次性退出**：基类 `finish()` 虽然返回 `handleContinuousMode()`，但项目的
   `releaseButton` 忽略该返回值，只设 `continuousMode = false` 并不会销毁工具。
   所以 `executeCommands` 末尾显式 `quit()`（`setActive(false)` + 工具栏按钮弹起）；
2. **激活重置**：第一次执行后状态机停留在 `End`，`sourceWires`/`offsetLength` 等
   全部残留。若不做清理，第二次选新曲线再点 Offset，会直接复用上一轮的 wire 与
   距离，表现成“始终偏移第一次选的曲线”。`onSetActive(true)` 中执行：

```cpp
void DrawSketchHandlerOffset::onSetActive(bool flag) {
    if (flag) {
        reset();                       // clears ShapeGeometry/lines, state back to SeekFirst
        sourceWires.clear();
        listOfGeoIds.clear();
        endpoint = Base::Vector2d(0.0, 0.0);
        onlySingleLines = true;
        deleteOriginal = false;
        offsetLengthSet = false;
        offsetLength = 0.0;
    }
    SupperClass::onSetActive(flag);
}
```

## 10. 注册与 UI

- 工具栏：`Moon/editor/Toolbar/sketchToolbar.cpp`
  `CreateCurveCommand(self, "DrawSketchHandlerOffset")`，加入互斥 `blackList`，
  图标为 FreeCAD 官方 `Sketcher_Offset.svg`（资源：`Moon/resource/icons/`，
  `resource.qrc` 的 `/widgets` 前缀）；
- 交互层：`GizmoRenderPass` 构造时 `mWidgets["DrawSketchHandlerOffset"]` 注册单例。

## 11. 已知限制 / TODO

- 椭圆、B-spline 的偏移结果不会转成几何（`CurveToPartGeometry` 返回空）；
- 混选“多段链 + 孤立边”时，只有整体全是单边才会用平面构造器，孤立边可能偏移失败；
- 开放链的偏移方向由 OCC 判定，与鼠标点击在哪一侧无关（与 FreeCAD 行为一致）；
- 未自动生成端点重合/相切与“偏移量”约束（项目的约束体系尚未接通 offset 约束）；
- 偏移后不自动清除原选中项；`D` 只负责删除选中的原始几何；
- 撤销/重做依赖上层命令系统，本工具未单独接入。

## 参考源码

- `Moon/Interactive/Widgets/DrawSketchHandlerOffset.h/.cpp`
- 框架：`Moon/Interactive/Widgets/DrawSketchDefaultHandler.h`
- FreeCAD 对照：
  `D:\Project\C++\FreeCAD\src\Mod\Sketcher\Gui\DrawSketchHandlerOffset.h`
