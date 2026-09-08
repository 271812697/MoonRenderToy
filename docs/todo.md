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
