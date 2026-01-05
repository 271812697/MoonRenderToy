#pragma once
#include <Maths/FVector2.h>
#include <Maths/FVector3.h>
namespace Core::SceneSystem{
	//Ray hit
	struct HitRes {
		Maths::FVector3 hitPoint;
		Maths::FVector2 hitUv;
		Maths::FVector3 hitNormal;
		int actorId = -1;
		int triangleId = -1;
	};
	//Rect Pick
	struct RectPickRes {
		int actorId = -1;
		int childId = -1;
		int triId = -1;
	};
	// Point Pick
	struct PointPickRes
	{
		int actorId = -1;
		int subMeshId = -1;

	};

	// 定义2D点结构体，简化坐标操作
	using Point2D = Maths::FVector2;

	/**
		* 向量叉乘计算函数
		* @param A 起点
		* @param B 第二个点
		* @param C 第三个点
		* @return 叉乘结果：(B-A) × (C-A)
		*/
	float cross(const Point2D& A, const Point2D& B, const Point2D& C);
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
		float rect_xmax, float rect_ymax);
	/**
		* 判断点是否在三角形内部（含边界，叉乘法）
		* @param Q 待判断点
		* @param P0 三角形顶点1
		* @param P1 三角形顶点2
		* @param P2 三角形顶点3
		* @return true：点在三角形内/边界上，false：点在三角形外
		*/
	bool isPointInTriangle(const Point2D& Q,
		const Point2D& P0, const Point2D& P1, const Point2D& P2);

	/**
		* 判断两条线段是否相交（含端点、含共线重叠，跨立实验）
		* @param A 线段1端点1
		* @param B 线段1端点2
		* @param C 线段2端点1
		* @param D 线段2端点2
		* @return true：线段相交/接触/共线重叠，false：线段不相交
		*/
	bool isSegmentIntersect(const Point2D& A, const Point2D& B,
		const Point2D& C, const Point2D& D);

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
		float rect_xmax, float rect_ymax);

	float pointToSegmentDistance(const Point2D& P, const Point2D& P0, const Point2D& P1);
}