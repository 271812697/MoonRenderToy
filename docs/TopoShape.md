# TopoShape 源码分析：CAD 形状封装与拓扑命名管线

> 本文基于 `MoonGeomerty/TopoShape.h`、`MoonGeomerty/TopoShape.cpp`、`MoonGeomerty/TopoShapeExpansion.cpp`、`MoonGeomerty/TopoShapeCache.h/.cpp` 的实际代码整理，重点剖析拓扑命名管线中的核心函数 `mapSubElement`。相关背景可配合 [ElementMap.md](./ElementMap.md) 阅读。

---

## 1. 概述

### 1.1 类定位

`Part::TopoShape` 是 FreeCAD 对 OpenCascade（OCCT）BRep 形状的封装类，它在一个类里同时承担三件事：

1. **承载几何**：持有 OCCT 的 `TopoDS_Shape`（通过 `_Shape` 成员）；
2. **承载拓扑命名**：继承 `Data::ComplexGeoData`，因而自带 `ElementMap`（元素映射表）、`Hasher`（字符串表）与 `Tag`（历史标签）；
3. **提供建模操作**：`makeXxx` 系列方法（建线、建面、拉伸、布尔、组壳等）在生成新形状的同时维护元素映射。

```cpp
class TopoShape : public Data::ComplexGeoData
```

### 1.2 与 ElementMap 的关系

`TopoShape` 是 `ElementMap` 最核心的**上层使用者**：

- 每个 `TopoShape` 经 `ComplexGeoData` 持有一张 `ElementMapPtr _elementMap`，记录“该形状子元素 ↔ 稳定映射名”的双向关系；
- 每次建模操作后，新形状通过 `mapSubElement` 把源形状的映射名继承过来，并编码本次操作的 postfix 与源 tag——这正是 `ElementMap` 文档中 `;:H<tag>:<len>,<type>` 历史链的**产生来源**。

---

## 2. 关键数据成员

### 2.1 成员总览

| 成员 | 类型 | 作用 |
| --- | --- | --- |
| `_Shape` | `ShapeProtector`（继承 `TopoDS_Shape`） | 受保护的 OCCT 形状句柄 |
| `Tag` | `long`（来自 `ComplexGeoData`，:481） | 形状所属对象的全局唯一标签，用作历史编码 |
| `Hasher` | `App::StringHasherRef`（来自 `ComplexGeoData`，:484） | 字符串表，用于映射名压缩 |
| `_elementMap` | `Data::ElementMapPtr`（来自 `ComplexGeoData`） | 元素映射表 |
| `_cache` / `_parentCache` | `std::shared_ptr<TopoShapeCache>` | 子形状索引与祖先缓存 |
| `_subLocation` | `TopLoc_Location` | 子形状相对父形状的位置 |

### 2.2 ShapeProtector：可变操作的“自动失效”机制

`_Shape` 不是裸 `TopoDS_Shape`，而是 `ShapeProtector`（`TopoShape.h:2864`）——一个包装类，目的是**保证 ElementMap 与缓存的一致性**：

- `Nullify()`：清空形状时同步 `resetElementMap()`、重置 `_cache` 与 `_parentCache`；
- `Orientation()` / `Reverse()` / `Complement()` 等会改变拓扑语义的操作：先 `flushElementMap()` 再改，改完重新 `initCache()`；
- `Location()` / `Move()` 只改放置，不影响元素映射与缓存，所以直接透传。

这样调用方无法通过底层 OCCT 句柄绕过命名维护，任何破坏性修改都会使映射/缓存自动失效重建。

### 2.3 Tag 与 Hasher

- `Tag`：产生该形状的文档对象 ID，`0` 表示未归属，`-1` 表示禁止映射；
- `Hasher`：与 `ElementMap::hasher` 必须保持一致（`resetElementMap` 会强制同步）。映射名里的 `#<hex>` 只有在同一张字符串表下才能互相解码。

---

## 3. TopoShapeCache：子形状索引缓存

`TopoShapeCache`（`TopoShapeCache.h:50`）为每个形状缓存两类信息：

### 3.1 Ancestry：子形状索引表

```cpp
class Ancestry {
    TopTools_IndexedMapOfShape shapes;   // 该类型全部子形状，1 基索引
    std::vector<TopoShape> topoShapes;   // 与 shapes 一一对应的 TopoShape
    std::array<AncestorInfo, TopAbs_SHAPE + 1> ancestors; // 祖先关系缓存
};
```

核心接口：

- `getAncestry(type)`：按类型懒加载，`TopExp::MapShapes(shape, type, shapes)` 填充索引表（`TopAbs_SHAPE` 时用 `TopoDS_Iterator` 枚举直接子形状）；
- `find(parent, subShape)`：`shapes.FindIndex(subShape)` 返回子形状在该类型索引表中的 1 基序号（0 = 不存在），带 Location 归一化；
- `find(parent, index)`：反查第 index 个子形状；
- `count()`：`shapes.Extent()`。

这套缓存让“形状 → 子元素索引”的转换是 O(1) 查找，且索引遵循 OCCT 拓扑遍历的规范顺序，**与命名序号无关**——这是 `mapSubElement` 能在两个形状之间建立元素对应的基础。

### 3.2 relations：形状关系缓存

`std::map<ShapeRelationKey, QVector<MappedElement>> relations` 缓存“给定映射名 + 追溯类型 → 相关元素”的结果（`ShapeRelationKey = MappedName + HistoryTraceType`）。`mapSubElement` 在开始时（`canMapElement` 内）会清空本形状的 relations，防止旧映射污染新结果。

---

## 4. 拓扑命名管线概览

一次典型建模操作（如拉伸、布尔、修型）后的命名流程：

```text
新形状 = 建模操作(源形状...)
   │
   ▼
makeShapeWithElementMap(结果形状, Mapper, 源形状列表, op)
   │   mapper 记录“源子形状 → 结果子形状”的对应
   ▼
mapSubElement(源形状, op)      ← 核心：继承并编码映射名
   │
   ▼
结果形状的 ElementMap 写入：
  <源元素映射名> + <op 操作码> + ;:H<源Tag>:<len>,<类型>
```

`makeShapeWithElementMap`（`TopoShapeExpansion.cpp:1408`）是大多数 `makESHAPE`/`makeXxx` 的公共入口，`mapSubElement` 在其中负责把 mapper 找到的元素对应翻译成 ElementMap 条目。**只要形状内容被修改或修复，就必须重新走这条管线**（见 `TopoShape.cpp:3276` 的注释与第 5.8 节）。

---

## 5. mapSubElement 详细分析

### 5.1 函数族与重载

| 函数 | 位置 | 作用 |
| --- | --- | --- |
| `mapSubElement(const TopoShape&, op, forceHasher)` | `TopoShapeExpansion.cpp:900` | 核心：单源形状映射 |
| `mapSubElementsTo(vector<TopoShape>&, op)` | :1002 | 反向：把自己作为源，映射到多个输出形状 |
| `mapSubElement(const vector<TopoShape>&, op)` | :1062 | 多源形状：复合体走子图路径，否则逐个串联 |
| `mapSubElementForShape`（私有） | :874 | 单形状映射的辅助封装 |
| `mapSubElementTypeForShape`（私有） | :817 | 按类型（Vertex/Edge/Face）逐元素映射 |

声明于 `TopoShape.h:1691-1693`。三个公开重载都无返回值，失败时静默返回（不抛异常）。

### 5.2 输入参数语义

- `other` / `shapes`：源形状（本形状由它们演变而来）；
- `op`：操作码字符串（如 `XTR` 拉伸、布尔类型、`SEWING` 缝合等），作为 postfix 编码进新名字；为空则不追加；
- `forceHasher`：强制同步哈希器，用于跨形状复用同一张字符串表的场景。

### 5.3 前置检查：canMapElement

```cpp
bool TopoShape::canMapElement(const TopoShape& other) const   // :734
{
    if (isNull() || other.isNull() || this == &other
        || other.Tag == -1 || Tag == -1) {
        return false;
    }
    if ((other.Tag == 0) && !other.elementMap(false)
        && !other.hasPendingElementMap()) {
        return false;
    }
    initCache();
    other.initCache();
    _cache->relations.clear();
    return true;
}
```

准入条件：

1. 双方形状非空、不是同一个对象、`Tag` 均非 `-1`；
2. 源形状必须有身份可追溯：`other.Tag != 0`，或它已有元素映射 / 有 pending map，否则无法形成历史链；
3. 初始化双方 cache，并清空本形状的 relations 缓存。

不满足则直接 return，本形状保持无映射状态。

### 5.4 快速路径：同构复制 copyElementMap

```cpp
if (!getElementMapSize(false) && this->_Shape.IsPartner(other._Shape)) {
    if (!this->Hasher) {
        this->Hasher = other.Hasher;
    }
    copyElementMap(other, op);
    return;
}
```

条件：本形状还没有映射，且与源形状是 **IsPartner**（OCCT 中共享底层 TShape 的伙伴关系，拓扑完全同构）。此时子元素必然一一对应，无需逐元素查索引，直接把源形状的整张 ElementMap 复制为子元素区间：

```cpp
// copyElementMap（:748 附近）
// 对 VERTEX / EDGE / FACE 各调用 checkSubshapeCount，构造 MappedChildElements：
child.indexedName = 类型,1;
child.offset = 0;
child.count = 子形状数量;
child.elementMap = topoShape.elementMap();
child.tag = (this->Tag != topoShape.Tag) ? topoShape.Tag : 0;
child.postfix = op;
// 然后 resetElementMap() + setMappedChildElements(children)
```

这是最廉价的路径，典型场景是同一形状的不同定位/副本。

### 5.5 主路径：逐类型、逐元素映射

核心循环（:900 起）对 `TopAbs_VERTEX / EDGE / FACE` 三种类型各执行一遍：

#### 5.5.1 取双方祖先索引

```cpp
auto& shapeMap = _cache->getAncestry(type);
auto& otherMap = other._cache->getAncestry(type);
if (!shapeMap.count() || !otherMap.count()) {
    continue;   // 某一方没有该类型子元素，跳过
}
```

#### 5.5.2 hasher 统一

```cpp
if (!forceHasher && other.Hasher) {
    forceHasher = true;
    checkHasher(other);
}
```

`checkHasher` 闭包保证本形状与源形状**共用同一张 StringHasher 表**：

- 本形状无 hasher → 直接继承 `other.Hasher`；
- 有但不同 → 若本形状已有映射本应抛错（`FC_THROWM` 当前被注释），否则警告后覆盖为源的 hasher。

这是硬性前提：映射名中的 `#<hex>` 只有同一张表才能互相解码。

#### 5.5.3 forward / reverse 方向自适应

```cpp
if (otherMap.count() <= shapeMap.count()) {  // 源形状子元素更少：以源为主
    forward = true;
    count = otherMap.count();
} else {                                     // 本形状子元素更少：以本形状为主
    forward = false;
    count = shapeMap.count();
}
```

对第 k 个元素：

- **forward**：`i = k`（源索引），`idx = shapeMap.find(_Shape, otherMap.find(other._Shape, k))`——先取源形状第 k 个子形状，再在本形状索引表里查它的序号；
- **reverse**：`idx = k`（本形状索引），`i = otherMap.find(other._Shape, shapeMap.find(_Shape, k))`——反向查源形状里的序号。

`find` 返回 0 说明拓扑对应不上（例如修型删掉的小边），`continue` 跳过。**两形状子元素数量/顺序不同也能工作**，因为对应关系建立在“共享底层 TShape”之上，而非序号巧合。

#### 5.5.4 继承源元素的全部映射名

```cpp
Data::IndexedName element = Data::IndexedName::fromConst(shapetype, idx);
for (auto& v : other.getElementMappedNames(
         Data::IndexedName::fromConst(shapetype, i), true)) {
    auto& name = v.first;
    auto& sids = v.second;
```

`getElementMappedNames`（`ComplexGeoData.cpp:286`）：

- 优先 `elementMap->findAll(element)`，返回源元素的**所有**映射名（一个元素可有多个名字，全部继承）；
- 源元素没有映射名时，若 `needUnmapped=true`，退化为 `MappedName(源索引名)`（如 `Face3`），即用原始名继续编码——保证映射链不因源元素缺名而断裂。

名字携带的 `sids` 若与当前 hasher 不兼容（`!sids[0].isFromSameHasher(Hasher)`），警告后清空，避免把属于其他字符串表的 ID 写进文档。

#### 5.5.5 编码并登记

```cpp
ensureElementMap()->encodeElementName(
    shapetype[0], name, ss, &sids, Tag, op, other.Tag);
elementMap()->setElementName(element, name, Tag, &sids);
```

- `encodeElementName`：基础名 = 源映射名，postfix = `op`，tag = `other.Tag`（源形状，即“上一历史步”），masterTag = 本形状 `Tag`。结果形如 `Face1;XTR;:H2f:8,F`；
- `setElementName`：把新名字登记进本形状的 ElementMap（双向映射），sids 一并保存，供将来保存文档时引用字符串表。

### 5.6 多形状 / 复合体路径

`mapSubElement(vector)`（:1062）先处理特殊情形：**本形状是 COMPOUND 且与 shapes 一一对应**（逐个 `IsPartner` 校验，顺序必须匹配）。此时不展开逐个元素，而是对 Vertex/Edge/Face 分别把每个源形状的贡献建成**连续子图区间**：

```cpp
child.indexedName = 类型,1;
child.offset = offset;        // 累加偏移
child.count = s.countSubShapes(类型);
child.elementMap = s.elementMap();
child.tag = s.Tag;
child.postfix = op;
```

然后一次性 `setMappedChildElements(children)`。这正是 ElementMap 的层级子图机制：复合体的每个直接子形状各挂一张子图，查询 `Vertex15` 时按 offset 定位到具体源形状的子图递归查找，避免把成百上千个元素展开成冗余映射。

校验失败（顺序不匹配）或本形状不是复合体时，回退为逐个串联：

```cpp
for (auto& shape : shapes) {
    mapSubElement(shape, op);
}
```

### 5.7 mapSubElementsTo：反向批量映射

```cpp
void TopoShape::mapSubElementsTo(std::vector<TopoShape>& shapes, const char* op) const
{
    for (auto& shape : shapes) {
        shape.mapSubElement(*this, op);
    }
}
```

把 this 作为源，映射到多个输出形状——用于一个操作产出多个结果的场景（分割、阵列等）。

### 5.8 实际调用场景

| 场景 | 位置 | 说明 |
| --- | --- | --- |
| 形状修复 | `TopoShape.cpp:3276` | `ShapeFix_Shape` 可能删除/修改子形状，必须重新 `makeShapeWithElementMap`（内部调 `mapSubElement`）重映射，否则引用索引跳跃（引用了 realthunder/FreeCAD#595：Sketch001 小边被删导致 Sketch002.ExternalEdge5 失效） |
| 建线/建面/闭线 | FaceMaker、makEWires 等 | 把输入线/面的映射传给生成物 |
| 拉伸 | `ExtrusionHelper.cpp:644` | `sourceWire.mapSubElement(shape)`，让拉伸体轮廓边继承草图的边名 |
| 组复合体 | `TopoShapeExpansion.cpp:2360` 附近 | 多形状版本走子图路径 |
| 组壳 | `TopoShapeExpansion.cpp:5840` | `tmp.mapSubElement(*this, op)` 继承面的映射 |

### 5.9 关键设计点

1. **不依赖序号巧合**：forward/reverse 通过共享 TShape 查索引，子元素增删后仍能对上的继续映射，对不上的静默跳过；
2. **缺名退化**：源元素无映射名时用原始名继续编码，历史链不断；
3. **hasher 必须统一**：新形状与源共用字符串表，否则 `#<hex>` 无法互解；
4. **多映射名全继承**：一个源元素的所有映射名都会编码进新形状；
5. **op 只作 postfix，tag 用 other.Tag**：历史链由“源形状的 tag”串联，与 `ElementMap::encodeElementName` 的语义完全对应；
6. **复合体走子图**：`setMappedChildElements` 建立层级映射，避免冗余展开；
7. **快速路径**：IsPartner 同构复制整张图，避免无谓的逐元素处理。

### 5.10 代码状态与注意项

- `mapSubElementForShape`（:874）目前**没有任何调用者**，像是重构中间产物（`mapSubElementTypeForShape` 上有 `TODO: Refactor ... to reduce complexity`，且 :817 与主路径 :900 存在明显重复逻辑）；
- hasher 不一致时的 `FC_THROWM` 被注释，当前只警告并清空 sids；
- `checkHasher` 中“已有映射却换 hasher”的报错同样被注释，属于遗留风险（与 `ElementMap.md` 第 7 节记录的注释代码风格一致）。

---

## 6. 与 ElementMap 的关系（呼应）

| ElementMap API | mapSubElement 中的使用 |
| --- | --- |
| `encodeElementName` | 编码 `op` postfix + `other.Tag` 历史段 |
| `setElementName` | 登记“本形状元素 → 继承名”的双向映射 |
| `setMappedChildElements`（内部 `addChildElements`） | 复合体路径建立层级子图 |
| `getElementMappedNames` / `findAll` | 读取源形状的全部映射名 |
| `hasher` | 与 `TopoShape::Hasher` 保持同一张字符串表 |

编码出的名字正是 `ElementMap.md` 中分析的结构：

```text
#94;:G0;XTR;:H19:8,F;:H1a,F;BND:-1:0;:H1b:10,F
```

其中 `;XTR` 之类的操作码来自 `op`，`;:H<tag>` 历史段由每次 `mapSubElement` 的 `other.Tag` 逐层串联；`getElementHistory` / `traceElement` 再沿这些 tag 反向追溯。

---

## 7. 总结

`TopoShape` 是“几何形状 + 拓扑命名”的统一载体：

- `_Shape`（ShapeProtector）保证任何形状修改都会同步失效映射与缓存；
- `TopoShapeCache::Ancestry` 提供按 OCCT 拓扑顺序的 O(1) 子形状索引；
- `mapSubElement` 把源形状的元素映射继承到新形状：同构时整图复制，异构时按类型逐元素查找对应、编码 `op` 与源 tag 并登记进 ElementMap，复合体则建立层级子图；
- 整个体系与 `ElementMap` 的编码、历史追溯、序列化能力闭环，构成了 FreeCAD 参数化建模中“重建后几何元素仍可被稳定引用”的完整基础。
