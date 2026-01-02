
#include <Core/SceneSystem/Intersection.h>
namespace Core::SceneSystem{

		// 全局浮点容错阈值（避免浮点计算精度误差导致的误判）
		const float EPS = 1e-7;

		/**
		 * 向量叉乘计算函数
		 * @param A 起点
		 * @param B 第二个点
		 * @param C 第三个点
		 * @return 叉乘结果：(B-A) × (C-A)
		 */
		float cross(const Point2D& A, const Point2D& B, const Point2D& C) {
			return (B.x - A.x) * (C.y - A.y) - (B.y - A.y) * (C.x - A.x);
		}

		/**
		 * 判断点是否在轴对齐矩形（AABB）内部（含边界）
		 * @param P 待判断点
		 * @param rect_xmin 矩形最小x坐标
		 * @param rect_ymin 矩形最小y坐标
		 * @param rect_xmax 矩形最大x坐标
		 * @param rect_ymax 矩形最大y坐标
		 * @return true：点在矩形内/边界上，false：点在矩形外
		 */
		bool isPointInAABB(const Point2D& P,
			float rect_xmin, float rect_ymin,
			float rect_xmax, float rect_ymax) {
			return (P.x >= rect_xmin - EPS) && (P.x <= rect_xmax + EPS) &&
				(P.y >= rect_ymin - EPS) && (P.y <= rect_ymax + EPS);
		}

		/**
		 * 判断点是否在三角形内部（含边界，叉乘法）
		 * @param Q 待判断点
		 * @param P0 三角形顶点1
		 * @param P1 三角形顶点2
		 * @param P2 三角形顶点3
		 * @return true：点在三角形内/边界上，false：点在三角形外
		 */
		bool isPointInTriangle(const Point2D& Q,
			const Point2D& P0, const Point2D& P1, const Point2D& P2) {
			float c1 = cross(P0, P1, Q);
			float c2 = cross(P1, P2, Q);
			float c3 = cross(P2, P0, Q);

			// 三者同号（均≥0 或 均≤0），容错EPS处理边界情况
			bool allNonNeg = (c1 >= -EPS) && (c2 >= -EPS) && (c3 >= -EPS);
			bool allNonPos = (c1 <= EPS) && (c2 <= EPS) && (c3 <= EPS);

			return allNonNeg || allNonPos;
		}

		/**
		 * 判断两条线段是否相交（含端点、含共线重叠，跨立实验）
		 * @param A 线段1端点1
		 * @param B 线段1端点2
		 * @param C 线段2端点1
		 * @param D 线段2端点2
		 * @return true：线段相交/接触/共线重叠，false：线段不相交
		 */
		bool isSegmentIntersect(const Point2D& A, const Point2D& B,
			const Point2D& C, const Point2D& D) {
			// 计算各叉乘结果
			float c1 = cross(A, B, C);
			float c2 = cross(A, B, D);
			float c3 = cross(C, D, A);
			float c4 = cross(C, D, B);

			// 条件1：互相跨立（端点在对方两侧或在对方线段上）
			bool crossCondition = (c1 * c2 <= EPS) && (c3 * c4 <= EPS);

			// 条件2：处理共线情况（判断投影是否重叠）
			bool collinear = (fabs(c1) < EPS) && (fabs(c2) < EPS);
			if (collinear) {
				// 辅助函数：获取线段在x/y轴上的投影范围
				auto getSegmentRange = [](const Point2D& P, const Point2D& Q) {
					std::pair<float, float> xRange = { std::min(P.x, Q.x), std::max(P.x, Q.x) };
					std::pair<float, float> yRange = { std::min(P.y, Q.y), std::max(P.y, Q.y) };
					return std::make_pair(xRange, yRange);
					};

				auto [abX, abY] = getSegmentRange(A, B);
				auto [cdX, cdY] = getSegmentRange(C, D);

				// 投影重叠判定（x和y范围均有交集）
				bool xOverlap = (abX.first <= cdX.second + EPS) && (cdX.first <= abX.second + EPS);
				bool yOverlap = (abY.first <= cdY.second + EPS) && (cdY.first <= abY.second + EPS);

				return xOverlap && yOverlap;
			}

			return crossCondition;
		}

		/**
		 * 核心函数：判断三角形与轴对齐矩形（AABB）是否相交
		 * @param P0 三角形顶点1
		 * @param P1 三角形顶点2
		 * @param P2 三角形顶点3
		 * @param rect_xmin 矩形最小x坐标
		 * @param rect_ymin 矩形最小y坐标
		 * @param rect_xmax 矩形最大x坐标
		 * @param rect_ymax 矩形最大y坐标
		 * @return true：相交（含包含、接触、重叠），false：不相交
		 */
		bool isTriangleAABBIntersect(const Point2D& P0, const Point2D& P1, const Point2D& P2,
			float rect_xmin, float rect_ymin,
			float rect_xmax, float rect_ymax) {
			// 步骤1：快速排斥测试（计算三角形的AABB包围盒）
			float tri_xmin = std::min({ P0.x, P1.x, P2.x });
			float tri_xmax = std::max({ P0.x, P1.x, P2.x });
			float tri_ymin = std::min({ P0.y, P1.y, P2.y });
			float tri_ymax = std::max({ P0.y, P1.y, P2.y });

			// 包围盒不相交的4种情况，直接返回false
			if (tri_xmax < rect_xmin - EPS || tri_xmin > rect_xmax + EPS ||
				tri_ymax < rect_ymin - EPS || tri_ymin > rect_ymax + EPS) {
				return false;
			}

			// 步骤2：场景1：判断三角形任意顶点是否在矩形内
			if (isPointInAABB(P0, rect_xmin, rect_ymin, rect_xmax, rect_ymax) ||
				isPointInAABB(P1, rect_xmin, rect_ymin, rect_xmax, rect_ymax) ||
				isPointInAABB(P2, rect_xmin, rect_ymin, rect_xmax, rect_ymax)) {
				return true;
			}

			// 步骤3：场景2：判断矩形任意顶点是否在三角形内
			Point2D rectPoints[4] = {
				Point2D(rect_xmin, rect_ymin),  // 矩形左下角
				Point2D(rect_xmax, rect_ymin),  // 矩形右下角
				Point2D(rect_xmax, rect_ymax),  // 矩形右上角
				Point2D(rect_xmin, rect_ymax)   // 矩形左上角
			};
			for (int i = 0; i < 4; ++i) {
				if (isPointInTriangle(rectPoints[i], P0, P1, P2)) {
					return true;
				}
			}

			// 步骤4：场景3：判断三角形边与矩形边是否相交
			// 定义三角形的3条边
			std::pair<Point2D, Point2D> triEdges[3] = {
				{P0, P1}, {P1, P2}, {P2, P0}
			};
			// 定义矩形的4条边
			std::pair<Point2D, Point2D> rectEdges[4] = {
				{rectPoints[0], rectPoints[1]},
				{rectPoints[1], rectPoints[2]},
				{rectPoints[2], rectPoints[3]},
				{rectPoints[3], rectPoints[0]}
			};

			// 遍历所有边对，判断是否有相交
			for (const auto& triEdge : triEdges) {
				for (const auto& rectEdge : rectEdges) {
					if (isSegmentIntersect(triEdge.first, triEdge.second,
						rectEdge.first, rectEdge.second)) {
						return true;
					}
				}
			}

			// 所有场景均不满足，返回不相交
			return false;
		}
	
}