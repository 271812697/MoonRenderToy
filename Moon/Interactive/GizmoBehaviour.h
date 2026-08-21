#pragma once
#include <Eigen/Eigen>
#include <Eigen/Core>
#include <Eigen/Dense>


namespace MOON {
	class GizmoAxisRotate
	{
	public:
		static constexpr float PLANE_DOT_EPS = 0.001f;
		static constexpr float RAY_T_MIN = 0.001f;
		static constexpr float PROJ_LEN_EPS = 1e-6f;
		static constexpr float ANGLE_EPS = 1e-6f;
		static constexpr int   DEFAULT_SEG = 32;

		GizmoAxisRotate() = default;
		GizmoAxisRotate(const Eigen::Vector3f& Axis, const Eigen::Vector3f& center)
			: m_axis(Axis.normalized()), m_origin(center)
		{
		}

		void startPick(const Eigen::Vector3f& Axis, const Eigen::Vector3f& center,
			const Eigen::Vector3f& srcDir, const Eigen::Vector3f& srcPos);
		bool applyDir(const Eigen::Vector3f& ray, const Eigen::Vector3f& eye, Eigen::Vector3f& outDir);

		bool applyPos(const Eigen::Vector3f& ray, const Eigen::Vector3f& eye, Eigen::Vector3f& outPos);
		std::vector<Eigen::Vector3f> getRotationArc(int segCount = DEFAULT_SEG) const;

		float getRotationAngle() const;
		float getRotationAngleDegree() const;
		void enableSnap(bool flag);
	private:
		bool computePlaneProj(const Eigen::Vector3f& ray, const Eigen::Vector3f& eye, Eigen::Vector3f& outProj, Eigen::Vector3f& hitPos) const;

		float computeAngle(const Eigen::Vector3f& a, const Eigen::Vector3f& b) const;

	private:
		Eigen::Vector3f m_axis = Eigen::Vector3f::UnitX();
		Eigen::Vector3f m_origin = Eigen::Vector3f::Zero();

		Eigen::Vector3f m_refDir;
		Eigen::Vector3f m_refPos;
		Eigen::Vector3f m_mouseStart;
		Eigen::Vector3f m_lastProj; // 新增：缓存上一帧鼠标投影

		float m_totalAngle = 0.f;   // 累积总角，范围无限制
		bool m_firstPick = true;
		bool m_enableSnap = true;
	};
	class GizmoPlaneTranslate
	{
	public:
		GizmoPlaneTranslate() = default;
		GizmoPlaneTranslate(const Eigen::Vector4f& planeEq, const Eigen::Vector3f& pos)
			: plane(planeEq), origin(pos) {
		}
		void startPick(const Eigen::Vector4f& planeEq, const Eigen::Vector3f& pos);

		void apply(const Eigen::Vector3f& ray, const Eigen::Vector3f& eye, Eigen::Vector3f& pos);
	private:
		Eigen::Vector4f plane = Eigen::Vector4f(0, 0, 1, 0);
		Eigen::Vector3f origin = Eigen::Vector3f::Zero();
		Eigen::Vector3f mOffset = Eigen::Vector3f::Zero();
		bool firstPick = true;
	};
	class GizmoAxisTranslate {
	public:
		GizmoAxisTranslate() = default;
		GizmoAxisTranslate(const Eigen::Vector3f& Axis, const Eigen::Vector3f& pos) :axis(Axis), origin(pos) {}
		void apply(const Eigen::Vector3f& ray, const Eigen::Vector3f& eye, Eigen::Vector3f& pos);
		void startPick(const Eigen::Vector3f& Axis, const Eigen::Vector3f& pos);
	private:
		Eigen::Vector3f axis = { 1,0,0 };
		Eigen::Vector3f origin = { 0,0,0 };
		Eigen::Vector3f mInitialOffset = { 0,0,0 };
		bool firstPick = true;
	};
}