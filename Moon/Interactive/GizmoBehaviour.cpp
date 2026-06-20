#include "Interactive/GizmoBehaviour.h"
namespace MOON {
	void GizmoAxisRotate::startPick(const Eigen::Vector3f& Axis, const Eigen::Vector3f& center,
		const Eigen::Vector3f& srcDir, const Eigen::Vector3f& srcPos)
	{
		m_axis = Axis.normalized();
		m_origin = center;
		m_refDir = srcDir;
		m_refPos = srcPos;

		m_totalAngle = 0.f;
		m_firstPick = true;
		m_mouseStart.setZero();
		m_lastProj.setZero();
	}

	bool GizmoAxisRotate::applyDir(const Eigen::Vector3f& ray, const Eigen::Vector3f& eye, Eigen::Vector3f& outDir)
	{
		Eigen::Vector3f currProj;
		Eigen::Vector3f currHit;
		if (!computePlaneProj(ray, eye, currProj, currHit))
			return false;

		if (m_firstPick)
		{
			m_mouseStart = currProj;
			m_lastProj = currProj;

			m_firstPick = false;
			m_refPos = currHit;
			outDir = m_refDir;
			m_totalAngle = 0.f;
			return true;
		}

		// 计算单帧增量（相邻两帧夹角，永远不会超过半圈）
		float delta = computeAngle(m_lastProj, currProj);
		m_totalAngle += delta;
		m_totalAngle = fmod(m_totalAngle, 3.14159265358979323846f * 2);

		float snapAngle = m_totalAngle;
		//degree snap
		if (m_enableSnap) {
			int degree = m_totalAngle * 180 / 3.14159265358979323846f;
			snapAngle = degree * 3.14159265358979323846f / 180.0f;
		}


		m_lastProj = currProj;

		Eigen::AngleAxisf rot(snapAngle, m_axis);
		outDir = rot * m_refDir;
		return true;
	}

	bool GizmoAxisRotate::applyPos(const Eigen::Vector3f& ray, const Eigen::Vector3f& eye, Eigen::Vector3f& outPos)
	{
		Eigen::Vector3f currProj;
		Eigen::Vector3f currHit;
		if (!computePlaneProj(ray, eye, currProj, currHit))
			return false;

		if (m_firstPick)
		{
			m_mouseStart = currProj;
			m_lastProj = currProj;
			m_firstPick = false;
			m_refPos = currHit;
			outPos = m_refPos;
			m_totalAngle = 0.f;
			return true;
		}

		float delta = computeAngle(m_lastProj, currProj);
		m_totalAngle += delta;
		m_lastProj = currProj;

		float snapAngle = m_totalAngle;
		//degree snap
		if (m_enableSnap) {
			int degree = m_totalAngle * 180 / 3.14159265358979323846f;
			snapAngle = degree * 3.14159265358979323846f / 180.0f;
		}

		Eigen::AngleAxisf rot(snapAngle, m_axis);
		Eigen::Vector3f offset = m_refPos - m_origin;
		outPos = m_origin + rot * offset;
		return true;
	}

	std::vector<Eigen::Vector3f> GizmoAxisRotate::getRotationArc(int segCount) const
	{
		std::vector<Eigen::Vector3f> arcPoints;
		if (segCount < 2 || std::fabs(m_totalAngle) < ANGLE_EPS)
			return arcPoints;

		float snapAngle = m_totalAngle;
		//degree snap
		if (m_enableSnap) {
			int degree = m_totalAngle * 180 / 3.14159265358979323846f;
			snapAngle = degree * 3.14159265358979323846f / 180.0f;
		}

		Eigen::Vector3f baseOffset = m_refPos - m_origin;
		float step = snapAngle / static_cast<float>(segCount);
		for (int i = 0; i <= segCount; ++i)
		{
			float ang = step * static_cast<float>(i);
			Eigen::AngleAxisf rotStep(ang, m_axis);
			arcPoints.push_back(m_origin + rotStep * baseOffset);
		}
		return arcPoints;
	}

	float GizmoAxisRotate::getRotationAngle() const {
		//degree snap
		if (m_enableSnap) {
			int degree = m_totalAngle * 180 / 3.14159265358979323846f;
			return  degree * 3.14159265358979323846f / 180.0f;
		}
		return m_totalAngle;
	}
	void GizmoAxisRotate::enableSnap(bool flag) {
		m_enableSnap = flag;
	}
	bool GizmoAxisRotate::computePlaneProj(const Eigen::Vector3f& ray, const Eigen::Vector3f& eye, Eigen::Vector3f& outProj, Eigen::Vector3f& hitPos) const
	{
		const Eigen::Vector3f n = m_axis;
		const Eigen::Vector3f planePt = m_origin;

		float denom = ray.dot(n);
		if (std::abs(denom) <= PLANE_DOT_EPS)
			return false;

		float t = (planePt - eye).dot(n) / denom;
		if (t <= RAY_T_MIN)
			return false;

		Eigen::Vector3f hit = eye + ray * t;
		Eigen::Vector3f hitRel = hit - planePt;

		outProj = hitRel - n * (hitRel.dot(n));
		float projLen = outProj.norm();
		if (projLen < PROJ_LEN_EPS)
			return false;
		outProj /= projLen;
		hitPos = hit;

		return true;
	}

	float GizmoAxisRotate::computeAngle(const Eigen::Vector3f& a, const Eigen::Vector3f& b) const
	{
		float dot = a.dot(b);
		float crossSign = m_axis.dot(a.cross(b));
		return std::atan2(crossSign, dot);
	}

	void GizmoPlaneTranslate::startPick(const Eigen::Vector4f& planeEq, const Eigen::Vector3f& pos)
	{
		plane = planeEq;
		origin = pos;
		firstPick = true;
	}

	void GizmoPlaneTranslate::apply(const Eigen::Vector3f& ray, const Eigen::Vector3f& eye, Eigen::Vector3f& pos)
	{
		Eigen::Vector3f n(plane.x(), plane.y(), plane.z());
		float d = plane.w();

		float denom = ray.dot(n);
		if (std::abs(denom) <= 0.001f)
			return;

		float t = -(eye.dot(n) + d) / denom;
		if (t <= 0.001f)
			return;

		Eigen::Vector3f hit = eye + ray * t;
		if (firstPick)
		{
			mOffset = origin - hit;
			firstPick = false;
		}
		pos = hit + mOffset;
	}
	void GizmoAxisTranslate::apply(const Eigen::Vector3f& ray, const Eigen::Vector3f& eye, Eigen::Vector3f& pos) {
		Eigen::Vector3f planeTangent = axis.cross(pos - eye);
		Eigen::Vector3f planeNormal = axis.cross(planeTangent);
		Eigen::Vector3f planePoint = origin;
		float denom = ray.dot(planeNormal);
		if (std::abs(denom) <= 0.001f)
			return;
		float t = (planePoint - eye).dot(planeNormal) / denom;
		if (t <= 0.001f)
			return;
		Eigen::Vector3f point = eye + ray * t;

		if (firstPick)
		{
			mInitialOffset = origin - point;
			firstPick = false;
		}
		Eigen::Vector3f translationVector = point - planePoint + mInitialOffset;
		pos = planePoint + axis * translationVector.dot(axis);
	}
	void GizmoAxisTranslate::startPick(const Eigen::Vector3f& Axis, const Eigen::Vector3f& pos) {
		axis = Axis;
		origin = pos;
		firstPick = true;
	}	
}