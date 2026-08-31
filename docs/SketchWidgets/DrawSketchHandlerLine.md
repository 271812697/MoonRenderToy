# DrawSketchHandlerLine：直线工具

## 1. 概述

两点画直线：点 1 起点、点 2 终点，实时预览线段。

```cpp
class DrawSketchHandlerLine : public DrawSketchDefaultHandler<DrawSketchHandlerLine,
                                                               StateMachines::TwoSeekEnd, 2,
                                                               LineConstructionMethod>
```

## 2. 构造方式枚举（重点注意）

```cpp
enum class LineConstructionMethod { OnePointLengthAngle, OnePointWidthHeight, TwoPoints, End };
```

构造函数默认 `OnePointLengthAngle`，但**当前实现并没有按构造方式分支**——`updateDataAndDrawToPosition`
里没有读取 `constructionMethod()`，实际行为等价于 `TwoPoints`。`OnePointLengthAngle /
OnePointWidthHeight` 是 FreeCAD 的“一点 + 长度角度/宽高”画法残留声明，尚未接线。

## 3. 交互与数据流

```cpp
// SeekFirst：记起点
startPoint = onSketchPos;
drawPositionAtCursor(onSketchPos);

// SeekSecond：记终点并刷新预览
endPoint = onSketchPos;
try { CreateAndDrawShapeGeometry(); }
catch (const Base::ValueError&) {}   // 两端点重合时会抛错，忽略即可
```

`createShape`：

```cpp
Base::Vector2d vecL = endPoint - startPoint;
length = vecL.Length();
if (length > Precision::Confusion())
    addLineToShapeGeometry({startPoint.x,startPoint.y,0}, {endPoint.x,endPoint.y,0}, false);
```

长度为 0 时不生成几何（也意味着 `canGoToNextMode` 没有专门拦截，但预览为空）。

## 4. 注意点

- `onUpdate` 里画了 `m_internal->editPoint` 的 12px 点，但 `editPoint` 从未被更新（恒为
  (0,0)），会在原点画一个多余的点——疑似从 Point 工具复制时的残留；
- `lengthSign / widthSign / capturedDirection` 是 OVP（在线参数）修正的残留成员，只有
  `onReset` 清空它们，没有实际逻辑；
- 提交走模板默认流程（`End` → `executeCommands` → `addGeometry` 深拷贝）。
