# DrawSketchHandlerSymmetry：对称工具

## 1. 概述

把选中的几何关于**一条参考直线**做镜像。

```cpp
class DrawSketchHandlerSymmetry : public DrawSketchDefaultHandler<DrawSketchHandlerSymmetry,
                                                                   StateMachines::OneSeekEnd, 0>
```

## 2. 流程

```
SeekFirst：悬停命中的曲线必须是 GeomLineSegment → refGeoId
canGoToNextMode：refGeoId == -1 则不允许推进
按一次 → End → 释放提交
```

`listOfGeoIds` 每帧从 `getSelectIds()` 刷新（被镜像的几何）。

## 3. 预览与提交

```cpp
// 预览
std::vector<Part::Geometry*> symGeos = obj->getSymmetric(listOfGeoIds, dummy1, dummy2, refGeoId);
for (auto* geo : symGeos) ShapeGeometry.push_back(深拷贝);

// 提交（不走基类 executeCommands）
void executeCommands() override {
    obj->addSymmetric(listOfGeoIds, refGeoId);
    if (deleteOriginal) deleteOriginalGeos();   // 恒 false
}
```

## 4. 注意点

- `createShape` 只填预览，提交用的是 `SketcherObj::addSymmetric` 专用接口，不经过
  `ShapeGeometry`；
- `deleteOriginal` 成员存在但没有任何地方置位，原件永远保留；
- 预览通过 `getSymmetric` 生成镜像副本，镜像结果正确性依赖 `SketcherObj` 的实现。
