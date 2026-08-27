# ElementMap 源码分析：FreeCAD 拓扑命名核心映射类

> 本文基于 `MoonGeomerty/ElementMap.h` 与 `MoonGeomerty/ElementMap.cpp` 的实际代码整理，并参考了同目录下 `MappedName.h`、`IndexedName.h`、`StringHasher.h`、`ElementNamingUtils.h` 等相关依赖，解释该类在 FreeCAD 新拓扑命名（Topological Naming）体系中的作用与工作原理。

---

## 1. 概述

### 1.1 类定位

`Data::ElementMap` 是 FreeCAD 新拓扑命名系统的核心数据结构。它的职责是**在一个形状（Shape）与它的父对象之间建立稳定、可追溯的几何元素命名映射**：

- 传统命名（如 `Face1`、`Edge2`）只反映元素在某个形状内的序号，参数变化导致重建后序号变化，名称随之失效；
- 新拓扑命名需要一种“语义名”（MappedName）：它编码了元素在**形状历史**中的来源，因此即使中间过程删除了若干特征，元素仍能被稳定地找到。

`ElementMap` 在代码中作为 `ComplexGeoData` 的 `_id` 属性使用。注释原文：

> This class provides for ComplexGeoData's ability to provide proper naming. Specifically, ComplexGeoData uses this class for its `_id` property.

### 1.2 解决的问题

1. **双向查找**：由原始元素名（`IndexedName`，如 `Face3`）得到映射名（`MappedName`），或反向由映射名得到原始元素名。
2. **历史编码**：映射名内嵌“tag”（形状历史标签，通常是产生该形状的对象的 ID），用于追溯元素在建模历史中的来源。
3. **名称压缩**：通过 `StringHasher`（字符串表，长字符串取 SHA1 哈希）把冗长的映射名缩短为整数 ID。
4. **层级映射**：形状本身是嵌套结构（Solid → Face → Wire → Edge → Vertex），因此子形状会递归地拥有自己的 `ElementMap`（child element map）。
5. **持久化**：支持把整张映射表序列化到文档流、再从文档流恢复。

---

## 2. 关键概念与类型

### 2.1 IndexedName（原始元素名）

`IndexedName` 是“类型名 + 序号”的组合，例如 `Face1`、`Edge42`。它做了内存优化：

- 类型字符串（`Face`、`Edge` 等）在内部静态表中共享存储，同类型元素只保存一份字符串；
- 每个实例只额外保存一个 `int index`。

这是映射表的“旧名称”一侧。

### 2.2 MappedName（映射名）

`MappedName` 是映射表的“新名称”一侧，内部由两部分组成：

- `data`：不可变的主数据段；
- `postfix`：可追加的后缀段（后续建模操作往里追加编码片段）。

二者在逻辑上拼接成一个连续字节数组。它大量使用 `QByteArray` 的隐式共享（写时复制），并通过 `fromRawData` 实现零拷贝切片，以降低名称拼接的开销。

一个典型的映射名形如：

```text
Face1;:H19:8,F;:H1a,F;BND:-1:0;:H1b:10,F
```

其中 `;` 是元素映射前缀（`ELEMENT_MAP_PREFIX`），`;:H...` 是 tag 编码片段（见第 4 节）。

### 2.3 StringID / StringHasher（字符串表）

`App::StringHasher` 是一张双向字符串表：

- 把任意字符串映射成唯一整数 ID；
- 超过阈值的可哈希字符串只存 SHA1 哈希，原文不保留（`isHashed()`）；
- `StringID` 支持把数据拆成前缀/后缀、文本/索引等结构，进一步提高共享率；
- `StringIDRef` 是带引用计数的句柄，名字在表中以 `#<hex>` 形式出现，例如 `#1b`。

`ElementMap` 里保存一个 `App::StringHasherRef hasher` 成员，用于名称的“哈希化”与“去哈希化”（`hashElementName` / `dehashElementName`）。

### 2.4 tag（历史标签）

`tag` 是一个 `long` 整数，通常是产生当前形状的那个对象的 ID（masterTag）。它在编码名称时被写入 `;:H<tag>:<len>,<type>` 片段：

- `<tag>`：形状历史中前一步所属对象的 ID；
- `<len>`：本片段之前那段“操作码”的长度；
- `<type>`：元素类型字符（`F` 面、`E` 边、`V` 顶点）。

tag 支持负数（用于元素消歧），也支持 0（表示省略）。

### 2.5 前缀/标记常量（ElementNamingUtils.h）

| 常量 | 值 | 含义 |
| --- | --- | --- |
| `ELEMENT_MAP_PREFIX` | `;` | 映射名开头标记 |
| `MAPPED_CHILD_ELEMENTS_PREFIX` | `;:R` | 子元素映射名前缀 |
| `POSTFIX_TAG` | `;:H` | tag 编码（十六进制） |
| `POSTFIX_DECIMAL_TAG` | `;:T` | 旧版十进制 tag 编码 |
| `POSTFIX_EXTERNAL_TAG` | `;:X` | 外部链接对象的 tag |
| `POSTFIX_CHILD` | `;:C` | 子元素消歧后缀 |
| `POSTFIX_DUPLICATE` | `;D` | 重复映射重命名后缀 |
| `MISSING_PREFIX` | `?` | 缺失元素前缀 |

---

## 3. 数据结构（ElementMap.h）

### 3.1 成员总览

```cpp
std::map<const char*, IndexedElements, CStringComp> indexedNames; // 原始名 -> 映射记录
std::map<MappedName, IndexedName, std::less<>>       mappedNames; // 映射名 -> 原始名
QHash<QByteArray, ChildMapInfo>                      childElements; // 子元素映射索引
std::size_t                                          childElementSize = 0;
mutable unsigned _id = 0;    // 保存/恢复时使用的去重 ID
App::StringHasherRef hasher; // 名称哈希器
```

三张核心表：

| 表 | 键 → 值 | 作用 |
| --- | --- | --- |
| `indexedNames` | 元素类型名（如 `Face`）→ `IndexedElements` | 按类型组织原始元素的所有映射记录与子元素区间 |
| `mappedNames` | `MappedName` → `IndexedName` | 反向索引：由映射名查回原始名（保证名字唯一） |
| `childElements` | 子映射名的 key（`QByteArray`）→ `ChildMapInfo` | 快速定位某个映射名前缀对应哪个子元素区间 |

### 3.2 IndexedElements 与 MappedNameRef

```cpp
struct IndexedElements
{
    std::deque<MappedNameRef> names;            // 每个原始元素对应一个 MappedNameRef（按索引排列）
    std::map<int, MappedChildElements> children; // 子元素区间（键为区间结束位置）
};

struct MappedNameRef
{
    MappedName name;                 // 映射名
    ElementIDRefs sids;              // 组成该名字的 StringID 引用（用于保存与去哈希）
    std::unique_ptr<MappedNameRef> next; // 链表：一个元素可有多个映射名
};
```

**设计规则**：一个元素（`IndexedName`）可以拥有多个映射名（链式 `next`），但一个映射名只能对应一个元素（`mappedNames` 是唯一键）。这与注释中的说明一致：

> An element can have multiple mapped names. However, a name can only be mapped to one element.

`ElementIDRefs` 即 `QVector<App::StringIDRef>`，保存该名字编码依赖的所有字符串 ID——序列化时必须连同这些 ID 一起写出，否则恢复后无法解码。

### 3.3 MappedChildElements（子元素区间）

```cpp
struct MappedChildElements
{
    IndexedName indexedName; // 子元素起始名（类型 + 起始索引）
    int count;               // 覆盖多少个连续子元素
    int offset;              // 父索引相对于子索引的偏移
    long tag;                // 子映射的 tag
    ElementMapPtr elementMap; // 递归的子 ElementMap（可为空）
    QByteArray postfix;      // 该区间共享的后缀（编码后的）
    ElementIDRefs sids;      // 相关的 StringID
};
```

它描述的是：**父元素索引区间 `[start, start+count)` 整体映射到一个子形状区间 `[indexedName, indexedName+count)`**。当子形状也有自己的映射表时，`elementMap` 非空，查询需要递归。

---

## 4. 核心 API 与工作原理

### 4.1 建立映射：setElementName / addName

入口是 `setElementName(element, name, masterTag, sid, overwrite)`：

1. 校验 `element` 与 `name` 非空，且不含非法字符（`.` 与空白字符）；
2. 调用 `addName` 尝试把 `(name, element)` 插入 `mappedNames`，并把名字追加到 `indexedNames[type].names[index]` 的链表中；
3. 若 `name` 已被其他元素占用（冲突）：
   - `overwrite=false`（默认）：不覆盖，转 4；
   - `overwrite=true`：先 `erase` 旧绑定再插入，循环直至成功；
4. 冲突时调用 `renameDuplicateElement`，生成一个消歧名再重试（最多 100 次）。

`renameDuplicateElement` 在名字上追加 `ELEMENT_MAP_PREFIX + "D" + hex(序号)`（如 `;D3f`），并用 `encodeElementName` 编码 tag。注意发布版本中这个序号来自随机数（`std::random_device` + `mt19937`），仅调试版本使用递增的 `index`，目的是避免攻击者/用户通过名称推断内部结构。

### 4.2 名称编码：encodeElementName

`encodeElementName(element_type, name, ss, sids, masterTag, postfix, tag, forceTag)` 负责把“上一步的名字 + 本次操作的后缀 + tag”编码成新的映射名：

1. 若有 `postfix` 且不以 `ELEMENT_MAP_PREFIX` 开头，先补上 `;`；
2. 决定是否必须写 tag：
   - `forceTag` 强制写；
   - 若 `tag == 0` 或 `tag == masterTag`（即 tag 没有新信息），尝试复用名称中已有的 tag，避免层层重复编码（防止历史链无限膨胀）；
3. 若提供了 `hasher`，调用 `hashElementName` 把长名称缩短成 `#<hex>`；
4. 最终追加 `;:H<tag>:<len>,<type>` 片段。

`hashElementName` 只对**含 `ELEMENT_MAP_PREFIX`** 的长名称做哈希（普通 `Face1` 没必要），并记录相关 `StringIDRef` 到 `sids`。`dehashElementName` 是反向操作：把 `#<hex>` 还原为真实文本；若对应 StringID 已被哈希（`isHashed()`，原文已丢弃），则无法还原，原样返回。

### 4.3 查询：双向 find

**由映射名查原始名** `find(const MappedName&, sids)`：

1. 先在 `mappedNames` 中精确查找；
2. 未命中时，尝试解析名称里的 tag 片段，取 tag 之前的部分作为 key 查 `childElements`，命中则递归进子 `elementMap` 查询，再叠加 `offset` 还原父索引；
3. 若命中普通映射，还会顺带把关联的 `sids` 返回给调用者。

**由原始名查映射名** `find(const IndexedName&, sids)`：

1. 在 `indexedNames[type].names` 中按索引直接取（`deque` 随机访问）；
2. 若该位置没有直接映射，用 `children.upper_bound(idx)` 找到覆盖该索引的最近子区间，递归子图查询后拼接 `postfix`。

子区间查询使用 `upper_bound` + 结束位置作键：`children` 按**区间结束位置**排序，`upper_bound(idx)` 得到第一个结束位置大于 `idx` 的区间，再校验 `起始位置 <= idx`，从而在 `O(log n)` 内定位覆盖区间。

### 4.4 删除：erase

两个重载：

- `erase(const MappedName&)`：从 `mappedNames` 删除条目，并从对应 `MappedNameRef` 链表中摘除；
- `erase(const IndexedName&)`：清空该索引对应的整个 `MappedNameRef` 链表，并从 `mappedNames` 删除其所有名字。

### 4.5 子元素映射：addChildElements / hashChildMaps

形状重建后，父元素（如面）会携带一批子元素（如边、顶点）。`addChildElements` 把子形状的映射“挂”到父图上：

1. **孙图展开（grand child map expansion）**：若子元素本身还带子图，先把当前区间按孙区间切分成多段，递归下沉，避免将来查询时产生超长的递归拼接名称；
2. **区间打包**：对每个子区间，先尝试生成共享 key（`encodeElementName`），并维护 `ChildMapInfo` 的引用计数与 `mapIndices` 消歧；
3. **阈值优化**：只有 `child.count >= 5` 或没有子图时才建立区间映射；否则逐元素调用 `setElementName` 展开成普通映射（避免小区间占用过多表项）；
4. 同一形状被映射多次（如 Draft 阵列）时，用额外的 `;:C<序号>` 后缀做消歧；
5. 最终以 `indexedName + offset + count` 为键插入 `children`，并把生成的 key 注册进 `childElements`，累加 `childElementSize`。

`hashChildMaps` 在保存前把子元素的长 postfix 哈希化（前缀 `;:R`），以缩小文档体积。

### 4.6 历史追踪：getElementHistory / traceElement

这两个函数是拓扑命名的灵魂——**通过名字里层层嵌套的 tag 追溯元素来源**。

`getElementHistory(name, masterTag, original, history)`：

- 解析最内层 tag；
- 逐层剥离 `;:H...` 片段并 `dehashElementName` 还原上一层名字；
- 返回最终 tag（最原始的一步），可选输出 `original` 名字与完整 `history` 列表；
- 若某层 tag 与当前 tag 不一致且不是 masterTag 的取反，说明历史链断裂，提前返回。

`traceElement(name, masterTag, cb)`：把同一逻辑做成回调驱动的遍历：

1. 先调用一次 `cb(name, len, encodedTag, masterTag)`；
2. 若名字里没有 tag（`pos < 0`）或遇到 `;:X`（外部链接对象），停止；
3. 循环：截取上一段名字 → `dehashElementName` → 解析新 tag → 回调；
4. **循环保护**：用 `tagSet` 记录已见过的 tag，重复出现即判定为循环映射并中断（注释引用了 assembly3 issue #968 的真实事故）；同时设置最多 50 层深度上限。

#### getElementHistory 逐行剖析

`getElementHistory`（`ElementMap.cpp:1291`）是历史追溯的“一次性收集”版本：逐层剥离 tag 片段，回溯元素来源，返回最初创建该元素的一致 tag，并可选输出还原后的原始名与完整历史链。

**名字结构先看明白**——以函数注释中的例子为例（`;:H` 即 `POSTFIX_TAG`）：

```text
#94;:G0;XTR;:H19:8,F;:H1a,F;BND:-1:0;:H1b:10,F
```

| 片段 | 含义 |
| --- | --- |
| `#94` | StringID 引用（哈希/压缩后的元素名，十六进制 id） |
| `;:G0`、`;XTR` | 建模操作码片段 |
| `;:H19:8,F` | tag 段：tag=`0x19`，长度=`8`，类型=`F`（面） |
| `;:H1a,F` | tag 段：tag=`0x1a`，长度省略（0），类型=`F` |
| `;BND:-1:0` | 又一个操作码片段 |
| `;:H1b:10,F` | 最外层 tag 段：tag=`0x1b`，长度=`0x10`，类型=`F` |

tag 段格式为 `;:H<tag>:<len>,<type>`（旧格式 `;:T` 为十进制）。`<len>` 表示本段之前那段“上一层名字”的字节长度，`<type>` 是 `F/E/V` 元素类型。**最右边的 tag 是最近的一层历史，向左越来越旧。**

**第一步：解析最外层 tag**

```cpp
int pos = name.findTagInElementName(&tag, &len, nullptr, nullptr, true);
```

`findTagInElementName` 内部用 `rfind` 找**最后一个** `;:H`，因此：

- `pos` = 最外层 `;:H` 的字节偏移；
- `tag` = 该层编码的 tag（`negative=true` 保留负号，负数用于元素消歧）；
- `len` = 本层 tag 之前那段“上一历史层名字”的长度，即 `name[0..len)` 是上一层的编码名，里面还嵌着更旧的 tag。

两个提前返回：

- `pos < 0`：名字里没有 tag，说明没有历史，`original` 原样输出，返回 `0`；
- `!original && !history`：调用者只想要 tag，不要求反推名字，直接返回 `tag`，省掉全部去哈希开销。

**第二步：去掉前缀标记**

```cpp
if (name.startsWith(ELEMENT_MAP_PREFIX))   // ";"
    ret = MappedName::fromRawData(name, ELEMENT_MAP_PREFIX_SIZE);
else
    ret = name;
```

`;` 只是映射名的标记前缀，不是语义内容，先剥掉，保证 `*original` 最终返回“裸名”。

**第三步：循环逐层剥离**

每次迭代剥离一层 tag，直到链断或没有 tag：

1. **校验长度**：`len` 必须落在 `(0, pos]` 区间内，否则名字损坏，`FC_WARN` 后返回 0（调用方应视为无历史）。

2. **子元素图特殊处理（`;:R`）**：

   ```cpp
   if (ret.startsWith(MAPPED_CHILD_ELEMENTS_PREFIX, len)) {
       MappedName tmp2 = MappedName::fromRawData(ret, len + POSTFIX_TAG_SIZE, pos - len - POSTFIX_TAG_SIZE);
       MappedName postfix = dehashElementName(tmp2);
       if (postfix != tmp2) {
           deHashed = true;
           ret = MappedName::fromRawData(ret, 0, len) + postfix;
       }
   }
   ```

   这个分支对应 `hashChildMaps()` 生成的子元素映射名，其结构为：

   ```text
   <上一层名字> ;:R <被哈希的子元素 postfix> ;:H<tag>:<len>,<type>
   ```

   上一层的边界（`len`）正好落在 `;:R` 上。此时不能对整个 `ret[0..len)` 去哈希，因为被哈希的内容在 **postfix 位置**（`;:R` 与 tag 之间），所以先把 `ret[len+3 .. pos)` 那段截出、单独 `dehashElementName`，再拼回 `ret = 上一层名字 + 还原后的 postfix`。

3. **常规去哈希**：`ret = dehashElementName(MappedName::fromRawData(ret, 0, len))`，把上一层的名字还原——`dehashElementName` 通过 `hasher->getID(id)` 把 `#<hex>` 查回真实文本；若对应 StringID 是 `isHashed()`（原文已丢弃、只剩 SHA1），则无法还原，原样返回。

4. **再解析一层，做一致性校验**：

   ```cpp
   pos = ret.findTagInElementName(&tag2, &len, nullptr, nullptr, true);
   if (pos < 0 || (tag2 != tag && tag2 != -tag && tag != masterTag && -tag != masterTag))
       return tag;
   tag = tag2;
   if (history) history->push_back(ret.copy());
   ```

   这是整个函数最微妙的判断：**何时继续剥下一层？** 四个“不匹配”全部成立才停止：

   - `tag2 != tag` 且 `tag2 != -tag`：上一层名字解析出的 tag 与当前层对不上（连符号取反都对不上），链断了；
   - 同时 `tag != masterTag` 且 `-tag != masterTag`：当前这层也不是调用者自己产生的。

   换言之：**链一致（同 tag 或取反）就继续；当前层属于调用者（masterTag）也继续**。后者对应同一对象多步建模——对象连续消费自己上一步的形状时 tag 会重复等于 `masterTag`，若不豁免，历史链会过早断裂。

   继续时更新 `tag = tag2`，并把还原后的 `ret.copy()` 压入 `history`（`copy()` 强制深拷贝，避免 `fromRawData` 产生的共享/裸数据被后续操作破坏）。

5. **终止性**：每轮至少消耗掉一个 tag 段（新 `pos` 严格小于旧 `pos`），循环必然结束，不会死循环。

**关键设计点**：

- **方向**：从最外层（最新）向最内层（最旧）走，返回最旧的一致 tag——即“最初创建该元素的对象 ID”，调用方可用它定位源对象；
- **负 tag**：`negative=true` 保留负号，一致性判断用 `tag2 != -tag` / `-tag != masterTag` 兼容消歧用的负 tag；
- **masterTag 豁免**：应对同一对象多步建模的 tag 重复；
- **与 `traceElement` 的关系**：二者是同一套剥离逻辑的两种形态——`getElementHistory` 一次性收集结果，`traceElement` 回调驱动遍历，后者额外有 `tagSet` 循环检测与 50 层深度上限；前者靠 tag 一致性自然终止；
- **失败语义**：名字损坏（`len` 非法）返回 0 而非抛异常；无 tag 也返回 0，调用方需自行区分“无历史”与“历史损坏”。

#### traceElement 逐行剖析

`traceElement`（`ElementMap.cpp:1348`）与 `getElementHistory` 是同一套“剥离 tag 历史链”逻辑的**回调驱动版本**：它不返回收集结果，而是每剥出一层就调用一次回调，由调用方决定是否提前终止。

**回调签名语义**（`TraceCallback`，定义于 `ElementMap.h`）：

```cpp
typedef std::function<bool(const MappedName&, int, long, long)> TraceCallback;
// (name, offset/len, encodedTag, tag) -> bool
```

按头文件注释：

- `name`：当前这一层的元素名（第一层是完整原名字，之后是去哈希后的历史名）；
- `len`（即注释里的 offset）：跳过“本层编码名”的偏移，也就是下一层名字的边界；
- `encodedTag`：本层名字里编码的 tag，通常是**上一历史步**的 tag；
- `tag`：当前这一层所属形状的 tag（即传入的 masterTag）。

回调返回 `true` 表示调用方想停止遍历。

**第一步：解析最外层 tag 并先回调一次**

```cpp
auto pos = name.findTagInElementName(&encodedTag, &len, nullptr, nullptr, true);
if (cb(name, len, encodedTag, masterTag) || pos < 0) {
    return;
}
```

用 `rfind` 找到最右边的 `;:H`：`encodedTag` 取到最外层 tag，`len` 是它之前那段上一层编码名的长度。**注意：回调总是先对原始名字调用一次**——即使 `pos < 0`（名字里根本没有 tag），`cb` 也会先被调用，然后才因短路返回。所以回调至少会看到顶层名字。

**第二步：外部链接检查**

```cpp
if (name.startsWith(POSTFIX_EXTERNAL_TAG, len)) {
    return;
}
```

`POSTFIX_EXTERNAL_TAG` 即 `;:X`。如果紧接在上一层编码名边界处是 `;:X`，说明这段历史来自外部链接对象，其名字用的是**外部字符串表**，用当前 hasher 去解码是错的，直接终止。

**第三步：初始化循环保护**

```cpp
std::set<long> tagSet;
if (masterTag) tagSet.insert(std::abs(masterTag));
if (encodedTag) tagSet.insert(std::abs(encodedTag));
names.push_back(name);
masterTag = encodedTag;
```

- `tagSet`：记录已出现过的 tag（取绝对值），用于循环映射检测；
- `names`：纯诊断用途，把每一层名字存下来，循环报警时打印；
- `masterTag = encodedTag`：进入更旧一层后，“当前形状的 tag”就变成上一层编码出的 tag。

**第四步：最多 50 层的剥离循环**

```cpp
for (int index = 0; index < 50; ++index) {
    if (!len || len > pos) {
        return;   // 名字损坏，静默终止
    }
    if (first) {
        first = false;
        size_t offset = 0;
        if (name.startsWith(ELEMENT_MAP_PREFIX)) offset = ELEMENT_MAP_PREFIX_SIZE;
        tmp = MappedName(name, offset, len);   // 第一轮：从原名字切片
    }
    else {
        tmp = MappedName(tmp, 0, len);         // 之后：从上一轮 tmp 头部切片
    }
    tmp = dehashElementName(tmp);              // #<hex> -> 真实文本
```

每次迭代把当前层 tag 之前的 `len` 字节切出来，这就是上一层的编码名；`dehashElementName` 把它还原成真实文本（StringID 丢失或本身已哈希时原样返回）。第一轮要从原名字里切（可能带 `;` 前缀，先跳过 1 字节），之后 `tmp` 的头部就是上一层名字，从 0 切即可。

```cpp
    names.push_back(tmp);
    encodedTag = 0;
    pos = tmp.findTagInElementName(&encodedTag, &len, nullptr, nullptr, true);
    if (pos >= 0 && tmp.startsWith(POSTFIX_EXTERNAL_TAG, len)) {
        break;   // 下一层是外部链接，不再解码
    }

    if (encodedTag && masterTag != std::abs(encodedTag)
        && !tagSet.insert(std::abs(encodedTag)).second) {
        // FC_WARN("circular element mapping") ... 打印 names
        break;   // 循环映射
    }

    if (cb(tmp, len, encodedTag, masterTag) || pos < 0) {
        return;  // 回调要求停止，或历史链耗尽
    }
    masterTag = encodedTag;
}
```

**循环检测的条件**：

- `encodedTag` 为 0：没有可检测的东西；
- `masterTag == abs(encodedTag)`：这是**正常的链式递进**——同一对象多步建模时 tag 会相邻重复，不视为循环；
- 只有当 tag 重复出现且**不是紧邻的上一步**（`masterTag` 与它不同）时，`tagSet.insert(...)` 才会返回 false，判定为循环映射，记录警告并中断。

这正是注释里提到的 assembly3 issue #968：没有对象上下文时，查错字符串表可能意外造成循环映射，所以用 tagSet + 50 层上限双保险。（仓库当前版本里这段日志代码被注释掉，只保留了 `break`。）

**用例子走一遍**——以 `#94;:G0;XTR;:H19:8,F;:H1a,F;BND:-1:0;:H1b:10,F` 为例（最外层 tag 是 `;:H1b`）：

| 轮次 | 回调收到 | encodedTag | masterTag |
| --- | --- | --- | --- |
| 初始 | 完整原名字 | `0x1b` | 调用者 tag |
| 1 | `Face1;:G0;XTR;:H19:8,F;:H1a,F`（`#94` 已还原） | `0x1a` | `0x1b` |
| 2 | `Face1;:G0;XTR;:H19:8,F` | `0x19` | `0x1a` |
| 3 | `Face1;:G0;XTR`（无 tag，pos<0） | `0` | `0x19` |

最后一轮回调后因 `pos < 0` 返回。整个过程中 `masterTag` 依次变为 `0x1b → 0x1a → 0x19`，调用方拿着每层的 tag 就能查对象、拼出完整历史。

**与 `getElementHistory` 的对比**：

| 维度 | getElementHistory | traceElement |
| --- | --- | --- |
| 返回值 | 最终一致 tag（long） | void |
| 结果输出 | `original` + `history` 集合 | 每层回调驱动 |
| 提前终止 | 无回调，靠返回 | 回调返回 `true` 即可停止 |
| `;:R` 子图 postfix 去哈希 | 有 | **无**（只去哈希名字本体） |
| `;:X` 外部链接检查 | 无 | 有（初始 + 循环内各一次） |
| 循环检测 | 隐式（tag 一致性） | 显式 tagSet + 日志 |
| 深度上限 | 无（结构上必然终止） | 50 层硬上限 |
| 非法 len | `FC_WARN` 后返回 0 | 静默 return |

一个值得注意的差异：`traceElement` 没有 `getElementHistory` 里那个 `;:R` 分支，因此对子元素映射（`hashChildMaps` 生成的 `;:R<哈希>` 后缀）不会还原 postfix 部分，只会还原名字部分——后续若要做子元素的完整历史拼接，这是个需要留意的局限。

### 4.7 序列化：save / restore

`save(stream)` 的流程：

1. `collectChildMaps` 递归收集所有子图到 `childMaps`（同时把图本身加入 `childMapSet` 获得去重编号），并收集所有 postfix 到 `postfixMap`/`postfixes`；
2. 写头部：`<id> PostfixCount <n>`，随后逐个写出 postfix 字符串；
3. 写 `MapCount <n>`，再按收集顺序逐个写每个图；
4. 每张图写 `ElementMap <index> <id> <typeCount>`，然后按类型写出：
   - `ChildCount` + 每个子区间（索引、offset、count、tag、子图编号、postfix、sids）；
   - `NameCount` + 每个映射名。名字按三种形式压缩写出：
     - `: <postfix表序号> . <索引>`：纯 `类型+索引` 名字（复用类型字符串表）；
     - `$ <名字>`：名字正文是一个已标记 StringID（只写 ID 对应文本）；
     - `; <名字>`：普通文本；
     - 之后跟 `. <postfix 表序号>` 与 `. <sid值>` 序列。

`restore(hasherRef, stream)` 是逆过程：

- 先读 id，若 `_idToElementMap[id]` 已有，直接复用（**去重**）；
- 读 postfix 表、递归读子图（子图引用编号指向 `childMaps[mapIndex-1]`）；
- 逐类型恢复 `children` 与 `names`，把 `#<hex>` 名字通过 `hasherRef->getID` 还原为 `StringIDRef`；
- 恢复时保留向后兼容：子元素 sids 按十进制解析（注释说明这是历史上无意的格式，为兼容而未修正），名字内 hex 按十六进制。

### 4.8 跨图去重：beforeSave 与两张静态表

由于层级图的存在，同一张 `ElementMap` 可能被多个对象引用，而部分加载又要求冗余存储。为了恢复时不浪费内存，用两个静态表做“一次性 id”：

```cpp
static std::unordered_map<const ElementMap*, unsigned> _elementMapToId; // 本进程内：图 -> id
static std::unordered_map<unsigned, ElementMapPtr>      _idToElementMap; // 恢复时：id -> 已建图
```

- `beforeSave` 为每张图分配 id 并递归标记其依赖的 StringID 为“已使用”（`sid.mark()`），供 `StringHasher::compact` 清理无用字符串；
- `restore` 读到重复 id 时跳过加载直接返回已有实例。

（`init()` 中原本连接文档保存/恢复信号的代码目前被整体注释掉，`inited` 永远为 false，即这两张静态表不再按文档生命周期自动清理，属于遗留状态。）

---

## 5. 设计要点与工程细节

### 5.1 内存效率

- `IndexedName` 的类型字符串全局共享；
- `MappedName` 的 data/postfix 分离 + QByteArray 隐式共享 + `fromRawData` 零拷贝切片；
- `StringHasher` 字符串表：同名只存一份，长名存 SHA1；
- 序列化时对类型名、postfix、StringID 建表去重，文档里不重复写长字符串。

### 5.2 稳定性与冲突处理

- 名字 → 元素是唯一映射，元素 → 名字允许多个（链式 `MappedNameRef`）；
- 冲突时重命名加随机消歧后缀，绝不静默覆盖（除非显式 `overwrite`）；
- tag 相同导致的重叠编码被限制在“只多编码一层”，防止历史链过度膨胀。

### 5.3 健壮性

- 递归查询有深度上限（50）；
- 历史追踪用 `tagSet` 检测循环映射；
- 恢复时对索引、偏移、子图编号做范围校验；
- 外部链接（`;:X`）与已哈希字符串被识别并安全跳过。

---

## 6. 使用流程示意

```text
建模操作（如拉伸、倒角）
        │
        ▼
  新形状产生（携带 masterTag = 本操作对象 ID）
        │
        ▼
  ElementMap::setElementName(Face5, 旧名字+新后缀, masterTag)
        │
        ▼
  encodeElementName → 追加 ";:H<tag>:<len>,F" 片段
        │
        ▼
  addName 登记双向映射（mappedNames / indexedNames）
        │
        ▼
  用户后续选择该面 → find("...;:H1a,F") → Face5
        │
        ▼
  traceElement 逐层去哈希、剥离 tag，还原建模历史
```

---

## 7. 遗留问题与 TODO

| 位置 | 内容 |
| --- | --- |
| `init()` | 连接文档 save/restore 信号的代码整体注释，静态表生命周期管理缺失（`inited` 恒为 false） |
| `save`/`restore` | 大量错误抛出被注释（`FC_THROWM`），失败时可能静默返回错误数据 |
| `hashChildMaps` | `pos > 10` 的魔法数字，注释 `TODO: What is this 10?` |
| `restore` | 子元素 sids 按十进制解析，为向后兼容保留（历史上写错为十进制） |
| 头部注释 | `std::enable_shared_from_this` 带 `TODO can remove shared_from_this?` |
| 全局注释 | 提到同一套去重技术未来可应用于 OCC shape 的共享恢复 |

---

## 8. 总结

`ElementMap` 的本质是一张**带历史编码、支持层级、可持久化的双向名称映射表**：

- `indexedNames`（元素 → 名字链表）与 `mappedNames`（名字 → 元素）构成双向索引；
- `MappedName` 的 `;:H tag` 片段记录形状历史来源，支持回溯；
- `childElements`/`children` 实现子形状递归映射，配合 `offset/count` 区间压缩大量冗余条目；
- `StringHasher` + SHA1 负责把长名称压缩成短 ID；
- 序列化通过建表去重和 id 复用，在“支持部分加载”与“节省内存”之间取得平衡。

它是 FreeCAD 参数化建模中“重建后几何元素仍可被稳定引用”这一能力的基石。
