# DrawSketchHandlerLineSet：多段线工具

## 1. 概述

最复杂的手写工具：**不继承 `DrawSketchDefaultHandler`**，直接继承 `DrawSketchHandler`，因为
多段线的核心语义是“下一段从上一段的端点续接，并可选相切/垂直过渡”，三点击状态机表达不了。

```cpp
class DrawSketchHandlerLineSet : public DrawSketchHandler
```

## 2. 四套模式枚举

```cpp
enum SELECT_MODE     { STATUS_SEEK_First, STATUS_SEEK_Second, STATUS_Do, STATUS_Close };
enum SEGMENT_MODE    { SEGMENT_MODE_Arc, SEGMENT_MODE_Line };
enum TRANSITION_MODE { TRANSITION_MODE_Free, TRANSITION_MODE_Tangent,
                       TRANSITION_MODE_Perpendicular_L, TRANSITION_MODE_Perpendicular_R };
enum SNAP_MODE       { SNAP_MODE_Free, SNAP_MODE_45Degree };
```

`M` 键按 FreeCAD 的固定顺序循环“线段/圆弧 × 相切/垂直”组合；`Ctrl` 按住时圆弧角度吸附 45°。

## 3. 关键数据

```cpp
std::vector<Base::Vector2d> EditCurve;   // 当前段预览点列
int firstCurve, previousCurve;           // 首段 id / 上一段 id（用于续接）
SketcherObj::PointPos firstPosId, previousPosId;
Base::Vector2d CenterPoint;  double startAngle, endAngle, arcRadius;   // 圆弧段参数
Base::Vector3d dirVec;       // 续接方向（来自上一段端点切线/法线）
bool firstsegment;           // 是否还没有任何段（决定右键是退出还是重置）
```

## 4. 点击流程（自建状态机）

### 4.1 按下：SeekFirst → SeekSecond → Do / Close

```cpp
STATUS_SEEK_First：
    EditCurve[0] = 光标；
    testSelect 命中已有线段/圆弧的端点 → 记录 previousCurve/previousPosId + 更新过渡方向
    Mode = STATUS_SEEK_Second

STATUS_SEEK_Second：
    与 EditCurve[0] 重合（双击同点）→ 连续模式重置
    否则 Mode = STATUS_Do；若再命中起点 → Mode = STATUS_Close（闭合）
```

### 4.2 释放：生成并提交一段

```cpp
STATUS_Do / STATUS_Close：
    Line 段：GeomLineSegment(EditCurve[0], EditCurve[1]) → addGeometry
    Arc 段  ：由切线方向 + 光标算 arcRadius/CenterPoint/startAngle/endAngle →
              GeomArcOfCircle → addGeometry
    更新 previousCurve/previousPosId/EditCurve → 进入下一段（STATUS_SEEK_Second）
    STATUS_Close：提交后重置整条多段线，回 STATUS_SEEK_First
```

注意多段线的几何是**在释放时逐段直接 `addGeometry`** 的，不走 `ShapeGeometry` 缓冲。

## 5. 预览：续接几何学

`mouseMove`（每帧）：

- **Line + Free**：`EditCurve[末尾] = 光标`；
- **Line + Tangent/Perpendicular**：把光标投影到上一段切线/法线方向
  （`EditCurve[1].ProjectToLine(...)`），实现“锁定方向”的橡皮筋；
- **Arc + Tangent/Perpendicular**：
  ```
  θ = 切线方向与 (光标−起点) 的夹角
  arcRadius = |光标−起点| / (2·sinθ)
  用三角形面积法判断圆心在切线哪一侧（决定正负半径）
  CenterPoint = 起点 + (r·Ty, −r·Tx)
  startAngle / endAngle 由起点、圆心、光标算；Snap 45° 时把 arcAngle 取整到 π/4
  EditCurve[1..29] 离散成 29 个圆弧点 + [30]=圆心 + [31]=起点 → drawEdit 预览
  ```

## 6. 退出/重置

```cpp
quit() {
    if (firstsegment) DrawSketchHandler::quit();   // 一个段都没画 → 真退出
    else { 连续模式重置：全部状态回默认，EditCurve.resize(2) }   // 丢弃半成品
}
```

`Esc`（onKeyRelease）也走 `quit()`。

## 7. 注意点

- 大量 FreeCAD 约束代码（相切/垂直/重合/角度约束的生成）被注释，**当前只产生几何、不产生
  约束**，`previousPosId` 等的约束用途是空的；
- `onKeyPress` 的 `M` 循环逻辑对“上一段是圆弧还是直线”有完整分支，是 FreeCAD 原版语义的
  保留，值得作为后续加约束时的参照；
- 圆弧预览离散 29 段 + 圆心 + 起点共 32 个点，`EditCurve[29]` 是圆弧最后一点。
