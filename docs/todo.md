# 文档补全记录

下列文档缺口已补齐：

## 草图约束文档

- [x] 约束求解后，参数和 GCS 对象如何更新回原来的几何 ——
  [SketchConstraints.md](SketchConstraints.md) §13/§14
  （`GeoDef`/`double*` 参数仓库/`param2geoelement`/`ConstrDef` 数据结构拆解，
  `applySolution → updateGeometry → extractGeometry` 回写链路与失败回滚）

## 草图绘制文档

- [x] 曲线绘制 —— [SketchRendering.md](SketchRendering.md) §2
  （离散化缓存、颜色/选中、构造虚线、自适应网格背景）
- [x] 标注绘制 —— [SketchRendering.md](SketchRendering.md) §3
  （约束图标锚点/样式/序号、尺寸与角度标注、标签拖动与拾取）

## 草图交互文档

- [x] 移动几何曲线（端点/边/圆心的不同编辑语义）——
  [SketchInteraction.md](SketchInteraction.md) §4
- [x] 几何曲线的拾取与吸附 —— [SketchInteraction.md](SketchInteraction.md) §1/§2

## 渲染

### 计划支持的功能

**剔除**

- [x] HZB 遮挡剔除（深度金字塔、上一帧深度、BVH 分层节点测试）——
  第一版已落地，原理与实现见 [HzbOcclusionCulling.md](HzbOcclusionCulling.md)；
  待办：视空间 bias、一帧延迟处理、统计可视化
- [x] 深度异步回读（PBO 环形缓冲，避免剔除引入同步 stall）——
  见 [HzbOcclusionCulling.md](HzbOcclusionCulling.md) §5.4
- [ ] 屏幕尺寸剔除
- [ ] LOD / 重要度预算（按屏幕占比 × 距离 × 语义重要度取舍）

**绘制提交**

- [ ] 64 位排序键（pass / pipeline / material / depth / mesh）
- [ ] 每对象数据入 SSBO（去掉逐 draw 的 uniform 上传与状态切换）
- [ ] 大量actor场景下，parsescene和filterdrawable的优化，开启遮挡剔除后，这两部分会占据比较长的时间
- [ ] 帧 Arena 分配（绘制列表、剔除结果不再逐帧 new/delete）

**HAL 能力**

- [ ] compute shader（新增 `COMPUTE` stage 与 dispatch）
- [ ] indirect draw / MDI（`DrawElementsIndirectCommand` + `glMultiDrawElementsIndirect`）
- [ ] stream compaction / prefix sum（生成紧凑可见列表与 indirect 参数）

**调试与可视化**

- [ ] 剔除统计与可视化（候选 / 视锥剔除 / HZB 剔除 / 实际 draw 计数）

### 其他

- [ ] 梳理一帧场景渲染的流程，以及可优化的方法
- [ ] 材质系统的重构
