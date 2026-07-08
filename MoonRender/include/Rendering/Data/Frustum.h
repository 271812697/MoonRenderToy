#pragma once
#include <array>
#include <Maths/FMatrix4.h>
#include <Maths/FTransform.h>

#include "Rendering/Geometry/BoundingSphere.h"
#include "Rendering/Resources/Model.h"
#include <Rendering/Settings/ECullingOptions.h>

namespace Rendering::Data
{
	class Frustum
	{
	public:
		void CalculateFrustum(const Maths::FMatrix4& p_viewProjection);
		bool PointInFrustum(float p_x, float p_y, float p_z) const;
		bool SphereInFrustum(float p_x, float p_y, float p_z, float p_radius) const;
		bool CubeInFrustum(float p_x, float p_y, float p_z, float p_size) const;
		bool BoundingSphereInFrustum(const Rendering::Geometry::BoundingSphere& p_boundingSphere, const Maths::FTransform& p_transform) const;
		bool IsMeshInFrustum(Rendering::Resources::Mesh& p_mesh, const Maths::FTransform& p_transform) const;
		std::vector<Rendering::Resources::Mesh*> GetMeshesInFrustum(
			 Rendering::Resources::Model& p_model,
			const Rendering::Geometry::BoundingSphere& p_modelBoundingSphere,
			const Maths::FTransform& p_modelTransform,
			Rendering::Settings::ECullingOptions p_cullingOptions
		) const;

		std::array<float, 4> GetNearPlane() const;
		std::array<float, 4> GetFarPlane() const;

	private:
		float m_frustum[6][4];
	};
}