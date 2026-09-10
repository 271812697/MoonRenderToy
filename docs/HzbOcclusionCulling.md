# HZB 遮挡剔除：实现与工作原理

> 相关文档：[mass-entity-render-perf.md](./mass-entity-render-perf.md)（海量实体渲染的总体优化路线）、
> [BatchedMesh.md](./BatchedMesh.md)（合批，链路的第一级压缩）
> **深度约定**：[DepthPrecision.md](./DepthPrecision.md)（本项目使用 **Reversed-Z** + 动态近远平面）
>
> 适用场景：大量 entity 都在视锥内、但彼此存在遮挡关系（装配体、密集零件）。
> 本文记录本项目中**已落地的最小可用实现**（第一版）：CPU 侧 BVH 分层测试 + GPU 侧深度金字塔 + 小网格回读。

---

## 1. 一句话原理

先由上一帧的不透明深度建立一张「**最远深度**」金字塔（Reversed-Z 下即逐级取 **min**），然后本帧用 BVH 自顶向下测试：
**如果某个节点整体都比它覆盖区域里最远的那个表面还远，那么这个节点一定被完全遮住了，整棵子树可以扔掉。**

```text
上一帧：不透明绘制 → 深度 → 2×2 逐级取 min（Reversed-Z）→ 小网格（≤64×64）
本帧  ：BVH 节点 AABB → 屏幕矩形 → 查网格拿最远深度 → 比较 → 整棵剪枝
```

---

## 2. 为什么是「最大深度」金字塔

### 2.1 判据推导

逐像素的遮挡判据很直白：物体 O 被完全遮挡 ⟺ 它投影覆盖的**每个像素**上已绘制的表面都比 O 的最近点更靠近相机。

但逐像素比较太贵，于是把屏幕切成 tile，用**一个保守值**代表整个 tile：取该 tile 内所有像素深度的**最大值**（最远的那个表面）。于是：

```text
if (tile 内最远表面比物体的最近点还近)     // 用深度值比较，见 §2.2
    → tile 里连最远的表面都比物体近 → 物体必然完全被遮挡 → 剔除
```

这就是经典的 HZB（Hierarchical Z-Buffer）遮挡剔除。金字塔按 2×2 逐级取「最远表面」构建（本项目 Reversed-Z 下即取 **min**），于是任意矩形区域只需查**少数几个 texel**（本实现取覆盖范围内最远的那一个）即可得到保守判断。

### 2.2 方向不能搞错

| 管线深度约定 | 深度值含义 | 「最远表面」= | 金字塔用 | 判据（`objNear` = 物体最近点） |
|---|---|---|---|---|
| 标准（近→0、远→1，`depthFunc=LESS`） | 值大 = 远 | max | **max** | `tileMax < objNear - bias` |
| **Reversed-Z（近→1、远→0，`depthFunc=GREATER`）——本项目** | 值大 = 近 | min | **min** | `tileMin > objNear + bias` |

本项目的 Reversed-Z 由两处共同构成：`GLBackend` 里 `glDepthRange(1.0, 0.0)`（反转 NDC→窗口深度），
`PipelineState` 默认 `depthFunc = GREATER`（比较方向同时反转），清屏深度为 **0**。详见
[DepthPrecision.md](./DepthPrecision.md)。

因此在 HZB 里：**金字塔取 min，物体最近点取各角点的 max**，判据是 `tileMin > objNear + bias`。
另外别和 **Hi-Z** 混淆：GPU 光栅化层次深度测试用的那张金字塔存的是**最近**深度，目的是加速逐像素 depth test；遮挡剔除存的是**最远**，方向正好相反。

### 2.3 为什么用「上一帧」的深度

本帧的深度要等绘制完才有，而剔除必须在提交绘制**之前**完成。用上一帧的深度：

1. 时序上成立（剔除 → 绘制 → 建立下一帧要用的金字塔）；
2. 可以完全避开 GPU→CPU 同步（配合异步回读时）；
3. 代价是**一帧延迟**：快速移动的物体会有一帧的误剔/漏剔。

---

## 3. 在渲染管线中的位置

```text
Shadows(10000) → Skybox(10001) → Gbuffer(19999) → Opaques(20000)
   → SectionCap(25000) → SectionContour(25001) → Transparents(30000)
   → 【HZB(35000) ← 本次新增】
   → Post-Process(40000) → Lines(40002) → UI(50000)
   → … → Last（Picking / Gizmo / Debug Actor）
```

新增 `ERenderPassOrder::HzbBuild = 35000U`，插在**不透明与剖面封盖之后、后处理之前**。

每帧的数据流：

```text
帧 N
 ├─ Opaques / SectionCap 写入 MSAA 缓冲（颜色 + 深度）
 ├─ Transparents：depth peeling，透明深度写在 mLayerFbo[i]，不污染主深度
 ├─ HZB pass：
 │    ① CopyFramebufferDepth(MSAA → 自建 DEPTH_COMPONENT32F 纹理)   ← 解析 MSAA
 │    ② 逐级 2×2 取 min（Reversed-Z），减半到 ≤ 64×64 的 R32F 网格
 │    ③ ReadPixels 小网格 → HzbCuller::SetGrid()
 └─ Post-Process 及之后（不受影响）
帧 N+1
 └─ SceneRenderer::FilterDrawables → HzbCuller::Cull(BVH) → 被遮挡的 actor 不进绘制列表
```

---

## 4. 深度从哪来（关键设计点）

### 4.1 不能直接用 `mBlendFbo` 的深度

`TransparentRenderPass` 确实把不透明深度解析进了 `mBlendFbo`：

```cpp
// TransparentRenderPass::Draw
if (drawables.transparents.size() > 0) {
    CopyFramebufferColor(msaa, mBlendFbo);
    CopyFramebufferDepth(msaa, mBlendFbo);   // ← 深度解析在这里
    // ... depth peeling ...
}
```

但整段逻辑包在 `if (transparents.size() > 0)` 里：**场景里没有透明物体时这段不会执行**，`mBlendFbo` 的深度就不是本帧数据。HZB 的输入不能依赖另一个 pass 的内部分支，所以本实现选择自己解析。

### 4.2 本实现的做法

`HzbBuildPass` 自建一张 `DEPTH_COMPONENT32F` 纹理（并配一个占位 COLOR 附件，因为 `CopyFramebufferDepth` 用 COLOR 附件取尺寸），每帧执行：

```cpp
::Core::Rendering::FramebufferUtil::CopyFramebufferDepth(msaaBuffer, m_resolveFbo);
```

内部是 `glBlitNamedFramebuffer(..., GL_DEPTH_BUFFER_BIT, GL_NEAREST)`，从多重采样深度 → 单采样深度即完成 **MSAA 解析**（该项目在 `TransparentRenderPass` 里已经在用同一套 blit 解析深度，路径是验证过的）。

### 4.3 为什么透明不会被误当作遮挡物

因为透明的深度写在 peeling 的层缓冲里（`mLayerFbo[i]`，`depthWriting=true`），而最终 blend 回 `mBlendFbo` 时 `blit.stateMask.depthWriting = false`。所以从 MSAA 缓冲解析出来的深度**只有不透明 + 剖面封盖**——正好是遮挡剔除期望的遮挡物集合（半透明面不应遮挡任何东西）。

---

## 5. 金字塔构建的实现

### 5.1 目标和格式

- 每级是一张 **R32F** 单通道纹理（`EInternalFormat::R32F` + `EFormat::RED`/`FLOAT`，`NEAREST` 过滤、`CLAMP_TO_EDGE`）；
- 每级尺寸为上一级的 1/2，直到宽高都 ≤ `m_maxGridSize`（默认 **64**），最多 16 级；
- 1920×1080 时约 5 级，总像素约为全屏的 1/3，开销可忽略。

### 5.2 reduce shader

`Resource/Moon/Data/Engine/Shaders/PostProcess/HzbReduce.ovfx`：输出 texel 覆盖输入的 2×2 区块，取四者中**最远**的那个 —— Reversed-Z 下即**最小值**。

```glsl
vec2 halfTexel = 0.5 / _InputResolution;
float a = texture(_InputTexture, TexCoords + vec2(-halfTexel.x, -halfTexel.y)).r;
float b = texture(_InputTexture, TexCoords + vec2( halfTexel.x, -halfTexel.y)).r;
float c = texture(_InputTexture, TexCoords + vec2(-halfTexel.x,  halfTexel.y)).r;
float d = texture(_InputTexture, TexCoords + vec2( halfTexel.x,  halfTexel.y)).r;
FRAGMENT_COLOR = vec4(min(min(a, b), min(c, d)), 0.0, 0.0, 1.0);
```

深度纹理用普通 `sampler2D` 采样即可（返回值在 `.r`），这与该项目 depth peeling 中采样深度纹理是同一套做法。
换成非 Reversed-Z 的管线时，这里必须改成 `max`（否则会把「最近的表面」当作遮挡物，导致误剔）。

### 5.3 两个绘制路径

引擎没有 compute shader（`EShaderType` 只有 `NONE/VERTEX/GEOMETRY/FRAGMENT`），所以金字塔用**全屏 fragment pass 逐级降采样**，复用现成基础设施：

| 步骤 | 输入 | 用法 |
|---|---|---|
| 第 1 级 | **深度纹理**（不是 FBO 的颜色附件） | `m_renderer.Present(*m_resolveDepth, m_reduceMaterial)`：该重载会把纹理绑成 `_InputTexture`，再在当前绑定的目标上画全屏 quad |
| 第 2..N 级 | 上一级的 **R32F 颜色** | `m_renderer.Blit(pso, 上一级, 本级, m_reduceMaterial, FILL_INPUT_TEXTURE \| UPDATE_VIEWPORT_SIZE)`——`Blit` 会把源的颜色附件绑成 `_InputTexture`（`BloomEffect` 降采样链的同一套写法，`BloomDownsampling.ovfx` 是最佳模板） |

注意 `Blit` 的 `RESIZE_DST_TO_MATCH_SRC` 不能开（各级尺寸本就不同），所以显式传 flag 而非 `DEFAULT`；视口在绘制前用 `SetViewport(0,0,levelW,levelH)` 设为当前级尺寸。

### 5.4 回读

只读**最小的那一级**（≤64×64 的 R32F = 最多 16KB）：

```cpp
m_levels.back().ReadPixels(0, 0, gridWidth, gridHeight,
    EPixelDataFormat::RED, EPixelDataType::FLOAT, depths.data());
m_culler->SetGrid(gridWidth, gridHeight, std::move(depths));
```

第一版是**同步**回读（每帧一次 stall，见 §9）；正式版应改成 PBO 三帧环形缓冲，读第 N−2 帧的结果，彻底消除同步。

### 5.5 一个必须避开的坑：别把 `Framebuffer` 放进会扩容的容器

`GLFramebuffer` 没有自定义拷贝/移动构造，而它的析构函数会 `glDeleteFramebuffers(1, &m_context.id)`。
把 `Framebuffer` **按值**放进 `std::vector` 时，扩容会「复制旧元素 → 销毁旧元素」，
于是旧元素的析构把 GL 名字删掉，而新元素仍持有那个**已删除的 id**，
之后对它 `Bind()` 就会报：

```text
GL_INVALID_OPERATION: Framebuffer name must be generated before being bound.
```

因此本实现的每级金字塔帧缓冲用 `std::vector<std::unique_ptr<Framebuffer>>` 持有
（移动 `unique_ptr` 不会触碰帧缓冲对象本身）。项目里 `SceneRenderer::mLayerFbo` 用
`resize(8)` 一次成型、之后不再扩容，所以没有暴露这个问题——但新增代码若在循环里
`emplace_back` 帧缓冲就会踩到。

---

## 6. CPU 侧：BVH 分层遮挡测试

### 6.1 数据结构

项目已有的场景 BVH 正好满足需求（原本服务于拾取与路径追踪）：

```cpp
struct Bvh::Node {
    bbox bounds;            // world space AABB
    NodeType type;          // kInternal | kLeaf
    union {
        struct { Node* lc; Node* rc; };              // 内部节点
        struct { int startidx; int numprims; };      // 叶子
    };
};
inline int const* Bvh::GetIndices();                 // 叶子 → 图元索引
```

`BvhService::m_sceneBvh` 按 `mSceneMeshInstances` 的顺序构建（`Process(boxs, sceneMeshes, instances)`），因此 `GetIndices()[i]` → `mSceneMeshInstances[i]` → `actorID`，直接给出「被遮挡的是哪个 actor」。

### 6.2 节点测试（保守性全部体现在这里）

```cpp
// 8 个角投影
for (each corner of node->bounds) {
    clip = viewProjection * vec4(corner, 1);
    if (clip.w <= 1e-5) return false;              // 跨近平面/在眼后 → 不剔
    depth = 0.5 - clip.z / clip.w * 0.5;           // Reversed-Z：近 1 远 0
    uv    = (clip.xy / clip.w) * 0.5 + 0.5;
    记录 closestDepth = max(各角点 depth) 与 uv 包围盒   // 值大 = 近
}
if (closestDepth <= 0 || closestDepth >= 1) return false;  // 落在近/远平面 → 不剔
if (矩形完全在屏幕外) return false;                 // 交给视锥剔除
矩形 clamp 到 [0,1] → 映射到 grid texel 范围（含边界扩一格）
occluderDepth = min(该范围内的 grid 值)             // 保守：取最远（值最小）
return occluderDepth > closestDepth + bias;         // true = 完全被遮挡
```

四处保守处理都是刻意为之，宁可漏剔也不能误剔（误剔 = 物体凭空消失）：

1. `clip.w <= 1e-5`（含近平面穿越）→ 不剔；
2. 深度落在 [0,1) 之外 → 不剔；
3. 屏幕矩形越界 → clamp 后**向外取整**，只会让测试更保守；
4. 遮挡值取范围内**最远**的那个（Reversed-Z 下为 **min**），而不是平均值或最近的表面；
5. 背景（未绘制像素）在 Reversed-Z 下是 **0**（远），所以覆盖到背景的区域永远不会误剔。

### 6.3 遍历与 actor 粒度

```cpp
stack.push(root);
while (!stack.empty()) {
    n = stack.pop();
    if (被遮挡(n)) {                      // 一次测试剪掉整棵子树
        if (n 是叶子) 把叶子内所有图元的 actorID 记入 occluded;
        continue;
    }
    if (n 是叶子) 把叶子内所有图元的 actorID 记入 visible;
    else { stack.push(n->lc); stack.push(n->rc); }
}
for (actorID : visible) occluded.erase(actorID);   // 一个 actor 任一部分可见 → 整体必须画
```

- **为什么要分层**：一次节点测试可以剪掉子树里成百上千个物体，测试成本从 O(物体数) 降到 O(可见簇 + 边界簇)。CAD 里「一个大零件挡住后面上千个小零件」正是在 BVH 上层一刀剪掉。
- **为什么要 actor 归一化**：BVH 的图元是 mesh 实例，一个 actor 可能由多个实例组成；只要有一个实例可见，actor 就必须绘制。
- **叶子粒度**影响剪枝效率（每叶 16~64 个图元较合适）；**BVH 变更后必须重建**（`BvhService::SetDirtyFlag` / 场景的 `reBuildBvh`），否则用过期 AABB 剔除会出错。

### 6.4 bias

当前是深度值上的固定偏置（默认 `0.0005`），用于抵消深度精度与一帧延迟带来的边界抖动。
更严谨的做法是换成**视空间偏置**（把窗口深度反投影回 view space 再加一个米级 epsilon），
因为透视投影下窗口深度与视空间深度是非线性的，固定 ε 在不同距离上的等效厚度并不一致
（Reversed-Z 已经把精度拉平了很多，这一点比常规管线轻）。

---

## 7. 与渲染管线的接入点

`SceneRenderer::FilterDrawables`（每帧一次，绘制列表生成前）先跑一次剔除：

```cpp
const auto& sceneDescriptor = GetDescriptor<SceneRenderer::SceneDescriptor>();
if (auto* bvhService = sceneDescriptor.scene.GetBvhService();
    bvhService != nullptr && bvhService->m_sceneBvh != nullptr)
{
    m_hzbCuller.Cull(*bvhService->m_sceneBvh,
                     bvhService->mSceneMeshInstances,
                     camera.GetViewProjectionMatrix(),
                     m_frameDescriptor.renderWidth,
                     m_frameDescriptor.renderHeight);
}
```

drawable 循环内（视锥测试之后）执行剔除：

```cpp
if (m_hzbCuller.IsOccluded(desc.actor.GetID())) continue;
```

pass 注册在 `SceneRenderer` 构造函数里，并把 culler 指针交给 pass：

```cpp
auto& hzbPass = AddPass<HzbBuildPass>("HZB", ERenderPassOrder::HzbBuild);
hzbPass.SetCuller(&m_hzbCuller);
```

**开关**：pass 名叫 `HZB`，因此它自动出现在 ImGui 编辑器的 **Settings → Passes** 列表里，可以直接勾掉做对比（旧 Qt 编辑器可用同一套 pass 开关机制）。

---

## 8. 调试与验证

**统计量**（`SceneRenderer::GetHzbStats()` / `HzbCuller::GetStats()`）：

| 字段 | 含义 |
|---|---|
| `visitedNodes` | 本帧实际测试过的 BVH 节点数 |
| `culledNodes` | 因遮挡被剪掉的节点数（含内部节点，能反映剪枝效率） |
| `occludedActors` | 最终被判为完全遮挡的 actor 数 |
| `gridWidth/Height` | 回读网格尺寸（正常应 ≤ 64） |
| `cullTimeMs` | CPU 侧剔除耗时 |
| `HzbBuildPass::GetLastBuildTimeMs()` | 金字塔构建 + 回读耗时 |

**验证方法**：

1. 在 Settings → Passes 里开关 `HZB`，对比画面与帧率：
   - 画面**必须一致**（出现物体消失说明有误剔 → 调大 bias 或先关掉）；
   - 帧率/CPU 提交耗时应有改善，否则说明当前场景没有可利用的遮挡关系（稀疏场景很常见）。
2. 观察统计：`culledNodes` 高但 `occludedActors` 低 → 剪的是空子树，收益有限；两者都高才是目标场景。
3. 旋转/平移相机时留意有无「闪烁」（一帧延迟导致的边界抖动）→ 需要 bias 调优或对移动物体跳过剔除。

---

## 9. 已知限制与下一步

| 项 | 现状 | 下一步 |
|---|---|---|
| 回读方式 | 每帧一次同步 `ReadPixels`（≤16KB，仍会 stall） | PBO 三帧环形缓冲 + fence，读 N−2 帧 |
| 一帧延迟 | 用上一帧深度，未做特殊处理 | 对「本帧发生位移」的 actor 跳过剔除 |
| bias | NDC 固定 ε（0.0005） | 改为视空间偏置 |
| 统计可视化 | 只有 API，未画到 UI | 挂到 ImGui Settings 面板（候选 / 视锥剔除 / HZB 剔除 / 实际 draw 四个计数） |
| BVH 时效 | 假定 `m_sceneBvh` 已是最新 | BVH dirty 时跳过剔除 |
| 剔除粒度 | actor（BVH 叶子 → actorID） | 需要更细粒度时下探到 mesh 实例 |
| 每帧分配 | `unordered_set` 每帧 clear/插入 | 帧 Arena + 稀疏位图 |
| GPU 化 | 无 compute，无法做 GPU 侧测试 | 补 compute stage + indirect draw 后，可改为 GPU 遍历 BVH 写 indirect 参数（见 [todo.md](./todo.md) 的「HAL 能力」组） |

---

## 10. 关键代码索引

| 文件 | 内容 |
|---|---|
| `MoonRender/include/Rendering/Settings/ERenderPassOrder.h` | 新增 `HzbBuild = 35000U` |
| `Resource/Moon/Data/Engine/Shaders/PostProcess/HzbReduce.ovfx` | 2×2 取**最远**（本项目 Reversed-Z 下为 min）的降采样 shader |
| `MoonRender/include/Core/Rendering/HzbCuller.h` + `src/Core/Rendering/HzbCuller.cpp` | 网格存储、BVH 分层测试、actor 归一化、统计 |
| `MoonRender/include/Core/Rendering/HzbBuildPass.h` + `src/Core/Rendering/HzbBuildPass.cpp` | MSAA 深度解析、金字塔构建、回读、网格下发 |
| `MoonRender/include/Core/Rendering/SceneRenderer.h` + `src/Core/Rendering/SceneRenderer.cpp` | pass 注册、`FilterDrawables` 接入、`GetHzbCuller` / `GetHzbStats` |

---

## 11. 小结

- **输入**：上一帧的**不透明**深度——自己从 MSAA 解析，不依赖 transparent pass 的分支；透明走 depth peeling，不参与遮挡。
- **金字塔**：R32F、逐级 2×2 取**最远**（本项目 Reversed-Z → **min**；非 Reversed-Z 管线应取 max），减半到 ≤64 网格；无 compute，用全屏 fragment pass 实现。
- **回读**：只读最小级（≤16KB），当前同步，后续换 PBO。
- **测试**：BVH 分层遍历，节点 AABB → 屏幕矩形 → 与范围内最远深度比较（Reversed-Z：`tileMin > objNearClosest + bias` 即被判为遮挡）；近平面/越界/深度异常一律保守不剔；结果按 actor 归一化。
- **接入**：`SceneRenderer::FilterDrawables` 里剔除；pass 名 `HZB`，可随时开关对比。
