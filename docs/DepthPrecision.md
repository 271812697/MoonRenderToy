# 大尺寸场景深度冲突的解决：动态近远平面

> 透视相机下，模型尺寸很大时，移动相机（旋转/平移）会在模型表面出现
> “破碎”、闪烁、不断变化的小面片——这是**深度精度不足导致的 z-fighting**。
>
> 本文说明根因、量化修复前后的差距，并给出解决方案与数据。
>
> 涉及源码：
> - `Moon/renderer/CameraController.h/.cpp`（`UpdateDepthRange`，核心改动）
> - `MoonRender/src/Rendering/Entities/Camera.cpp`（`ProjectionFitToSphere` 远平面修复）
> - `MoonRender/src/Core/Rendering/FramebufferUtil.cpp`（深度缓冲格式）

---

## 1. 现象

透视相机下，把相机放到一个尺寸很大的模型附近（比如 fit 之后），**只要相机一
动**，模型表面就会出现随机变化的“破碎”区域。这不是网格问题，而是远处两个
本应分开的面，在深度缓冲里**分辨不出前后**，每帧交替获胜，表现为闪烁/破碎。

![窗口深度的非线性映射](images/depth_mapping.svg)

---

## 2. 原理：为什么大模型会碎

### 2.1 深度缓冲的精度模型

OpenGL 深度缓冲存的是**窗口深度 w ∈ [0,1]**，透视投影下 w 与视图深度 Z
（相机到物体的距离）是**非线性**关系：

```
w(Z) = ½ + (f+n)/(2(f−n)) − fn/((f−n)·Z)
```

对 Z 求导：

```
dw/dZ = fn/((f−n)·Z²) ≈ n/Z²        （当 f >> n）
```

深度缓冲是离散的（float32 有 24 位有效尾数，最小间隔 δw ≈ 2⁻²⁴ ≈ 5.96e-8），
所以某个距离 Z 处**能可靠分辨的最小深度差**为：

```
δZ ≈ δw / (dw/dZ) ≈ Z² / n · 2⁻²⁴
```

两个关键结论：

1. **δZ 与距离的平方成正比**——越远越差；
2. **δZ 与近平面 n 成反比**——near 越小，远处精度越差。

这也解释了为什么**已经是 float32 深度**（本项目主 MSAA framebuffer 用
`DEPTH32F_STENCIL8`）仍然会碎：格式只是增加位宽，非线性的分布没有变，
坏点仍然集中在远处。

### 2.2 修复前本项目的数据

修复前：`near = 0.1`（固定），`far = max(距离 + 半径, 1200)`（fit 大模型时
可能到几十万甚至上百万）。代入公式，**远平面附近的 δZ**：

```
δZ(far) = far² / 0.1 × 5.96e-8 = far² × 5.96e-7
```

| far | 远平面 δZ（最小可分深度差） | 意味着 |
| --- | --- | --- |
| 1,000 | 0.6 单位 | 表面间隔 < 0.6 就开始打架 |
| 10,000 | 60 单位 | 60 单位内的共面/近面全碎 |
| 100,000 | 5,960 单位 | 几乎所有相邻特征都 z-fight |
| 1,000,000 | 596,000 单位 | 完全不可用 |

这就是“模型一大，一动相机就碎”的直接原因：远处可分辨的最小深度差达到了
几千个单位，远大于模型上细小的面/倒角间距。

---

## 3. 解决方案：动态近远平面

### 3.1 思路

既然 δZ ∝ 1/near，**把 near 从固定 0.1 提升到与相机距离成比例的值**，就能成数量级
地改善远处精度；同时 near 又不能无限大（放大/贴近模型时会裁掉近处几何），所以
**按当前相机到场景的距离动态设置**，远近都能兼顾。

每帧执行（`CameraController::UpdateDepthRange`，在 `HandleInputs` 开头调用）：

```cpp
void ::Editor::Core::CameraController::UpdateDepthRange()
{
    Maths::FVector3 sceneCenter = m_view.GetRoaterCenter();
    float sceneRadius = 0.0f;
    if (auto* bvh = m_view.GetScene()->GetBvh()) {
        const auto& b = bvh->m_bounds;
        sceneCenter = (b.pmin + b.pmax) * 0.5f;
        sceneRadius = Maths::FVector3::Length(b.pmax - b.pmin) * 0.5f;
    }
    const float dist = Maths::FVector3::Length(m_camera.GetPosition() - sceneCenter);
    const float far  = std::max(dist + sceneRadius * 2.0f, 1200.0f);
    const float near = std::max(dist * 0.01f, 0.001f);   // near = 1% 的相机距离
    m_camera.SetNear(near);
    m_camera.SetFar(far);
}
```

`near = dist × 0.01` 的含义：

- 相机在 fit 距离附近时，`dist ≈ far/2`，于是 `near ≈ far × 0.005`，near/far 比例
  从原来的 1:1,000,000 收紧到约 1:200；
- 缩放贴近模型时 `dist` 变小，`near` 同步变小，不会裁掉眼前几何；
- 平移/旋转时每帧重算，始终跟随当前视野。

### 3.2 为什么有效

代入 `n = dist × 0.01 ≈ far × 0.005`：

```
δZ(far) = far² / (far × 0.005) × 5.96e-8 = far × 1.19e-5
```

远平面精度从 **δZ ∝ far²** 变成 **δZ ∝ far**——这是本质变化：模型再大，
精度只线性下降，而不是平方恶化。

![修复前后精度对比](images/depth_precision_chart.svg)

---

## 4. 修复前后数据对比

同一套公式、同一批 far 取值：

| far | 修复前 δZ（near=0.1） | 修复后 δZ（near=dist×0.01） | 提升倍数 |
| --- | --- | --- | --- |
| 1,000 | 0.596 单位 | 0.0119 单位 | ≈ 50× |
| 10,000 | 59.6 单位 | 0.119 单位 | ≈ 500× |
| 100,000 | 5,960 单位 | 1.19 单位 | ≈ 5,000× |
| 1,000,000 | 596,000 单位 | 11.9 单位 | ≈ 50,000× |

以 far = 100,000 为例：

- 修复前：远平面附近最小可分深度差 **≈ 6000 单位**——模型上任何小于 6000 单位的
  台阶/倒角/相邻面都会 z-fight；
- 修复后：**≈ 1.2 单位**——只有小于 1.2 单位的超细特征才可能打架（一般 CAD 特征
  远大于此）。

提升量 = `far × 0.05`：**模型越大，收益越大**（2~4 个数量级）。

---

## 5. 边界与后续

**本方案的边界**

- 它把 near 提到距离的 1%，当用户**贴到离模型 1% 距离以内**时仍可能裁剪近处几何；
- 极端超大模型（far > 10⁶）下，修复后远平面 δZ 仍有十几个单位，细特征仍可能闪。

### 5.1 另一条路：reversed-Z 深度缓冲（根治）

动态近远平面是把“精度缺口”压小；**reversed-Z 是把 float32 的精度放到最需要的地方**。

**原理**

float32 的尾数精度不是均匀的：**数值越接近 1，相对间隔越小**（[0.5, 1) 区间 ULP
恒为 2⁻²⁴）。标准深度把远处映射到窗口深度 ≈ 1 附近——但远处恰恰是导数最平的地方，
两者叠加最差。反转之后：

- 近平面 → 窗口深度 1，远平面 → 窗口深度 0（`glDepthRange(1, 0)`）；
- 远处（大 Z）落在 float32 最精细的 [0.5, 1) 区间；
- 深度函数从 `LESS` 改为 `GREATER`，清屏深度从 1 改为 0。

![Reversed-Z 映射](images/depth_reversed_mapping.svg)

精度变为**相对精度**：

```
δZ ≈ Z × 2⁻²⁴ ≈ Z × 6e-8
```

**与 near/far 比例无关**——即使 near/far 是 1:10⁶，远平面 δZ 也只有
`far × 6e-8`。

**对比（far = 100,000 为例）**

| 方案 | 远平面 δZ | 相对修复前 |
| --- | --- | --- |
| 修复前（near=0.1） | ≈ 5,960 单位 | 1× |
| 动态近远平面 | ≈ 1.19 单位 | ≈ 5,000× |
| reversed-Z（float32） | ≈ 0.006 单位 | ≈ 1,000,000× |

相比动态近远平面，还能再提升 **≈ 200 倍**。

**本项目需要改动的点**

1. GL 后端：`glDepthRange(1.0, 0.0)`，清屏时 `glClearDepth(0.0)`
   （`MoonRender/src/Rendering/HAL/OpenGL/GLBackend.cpp`）；
2. 默认深度函数 `LESS` → `GREATER`
   （`MoonRender/src/Rendering/Data/PipelineState.cpp`），以及 `SceneRenderer.cpp`
   里三处显式 `LESS_EQUAL` → `GREATER_EQUAL`（线 pass、剖切轮廓）；
3. 阴影比较反转：`Shadow.ovfxh` 的 `SampleShadow` 里
   `step(projCoords.z - bias, depth)` 方向要翻，`projCoords.z > 1.0` 的裁剪判断
   也要相应调整；
4. 深度剥离 shader（`GeomertySurface.ovfx` / `SectionFace.ovfx` 的 `Transparents`
   块）：窗口深度反算从 `raw * 2 - 1` 改为 `(1 - raw) * 2 - 1`，剥离层深度纹理的
   border color 从 1（far）改为 0；
5. 拾取、剖切奇偶、SSAO/Gbuffer 不受影响：它们要么用位置缓冲，要么只改深度函数
   方向即可自动一致。

**风险与建议**

- 改动横跨 GL 状态、阴影、透明剥离三块，而透明剥离（`Transparents`）和阴影是
  刚修过/正在用的功能，回归风险高；
- 建议**独立分支/独立任务**做，配一个“透明+阴影+剖切”的回归清单；
- 与动态近远平面**不冲突**：reversed-Z 后 near/far 比例不再影响精度，但收紧
  比例仍然没有坏处，可以两个都保留。

---

## 6. 涉及文件

- `Moon/renderer/CameraController.h/.cpp`：新增 `UpdateDepthRange()`，每帧动态设置
  near/far（本方案核心）；
- `MoonRender/src/Rendering/Entities/Camera.cpp`：`ProjectionFitToSphere` 的 far 修正
  （`distance + radius`），保证 fit 后模型整体在 far 平面内，不先被远裁剪切掉。
