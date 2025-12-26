#pragma once
#include <Rendering/HAL/VertexArray.h>
#include <Rendering/Geometry/BoundingSphere.h>
#include <Rendering/Geometry/bbox.h>

namespace Rendering::Resources
{
	/**
	* Interface for any mesh
	*/
	class IMesh
	{
	public:
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual uint32_t GetVertexCount() const = 0;
		virtual uint32_t GetIndexCount() const = 0;
		virtual const Rendering::Geometry::BoundingSphere& GetBoundingSphere() const = 0;
		virtual const Rendering::Geometry::bbox& GetBoundingBox()const=0;
	};
}