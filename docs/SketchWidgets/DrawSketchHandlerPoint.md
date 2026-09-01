# DrawSketchHandlerPoint：点工具

## 1. 概述

最简的模板工具：一次点击在草图上放一个点。用来理解 `DrawSketchDefaultHandler` 的完整流程
再合适不过——它只覆写了三个钩子里的两个（没有覆写 `canGoToNextMode`，永远允许推进）。

```cpp
class DrawSketchHandlerPoint : public DrawSketchDefaultHandler<DrawSketchHandlerPoint,
                                                               StateMachines::OneSeekEnd, 1>
```

## 2. 状态机

`OneSeekEnd { SeekFirst, End }`：点一次 → `SeekFirst → End` → 释放时提交。

## 3. 关键数据

```cpp
class Internal { Base::Vector2d editPoint; };   // 光标当前所在的草图坐标
```

## 4. 交互与数据流

```cpp
void updateDataAndDrawToPosition(Base::Vector2d onSketchPos) {
    if (state() == SeekFirst) {
        m_internal->editPoint = onSketchPos;   // ① 记录光标位置
        drawPositionAtCursor(onSketchPos);     // ② 显示坐标浮标
        CreateAndDrawShapeGeometry();          // ③ 刷新预览（生成临时点几何）
    }
}
```

`onUpdate()` 除了画框架预览，还额外用 12px 的点强调当前编辑点：

```cpp
DrawSketchHandler::onUpdate();                              // 吸附点/坐标/折线
renderer->drawPoint(plane.valueEigen(m_internal->editPoint), 12);   // 编辑点
```

## 5. 几何生成与提交

```cpp
void createShape(bool onlyeditoutline) {
    ShapeGeometry.clear();
    addPointToShapeGeometry(Base::Vector3d(editPoint.x, editPoint.y, 0.0), false);
}
```

按下 → 状态到 `End`；释放 → `finish()` → `executeCommands()` → `SketcherObj::addGeometry`。

## 6. 注意点

- `addPointToShapeGeometry(..., false)` 的第二个参数是 `constructionMode`，目前不生效（见主文档）；
- 没有覆写 `canGoToNextMode`，任何点击都会推进状态；
- 连续模式下每次提交后自动重置，可以连续放点；右键在 `SeekFirst` 时退出工具。
