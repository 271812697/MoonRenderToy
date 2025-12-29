#pragma once
#include <memory>
#include <vector>
#include <Rendering/HAL/IndexBuffer.h>
#include <Rendering/HAL/VertexArray.h>
#include <Rendering/HAL/VertexBuffer.h>
#include <Rendering/Geometry/Vertex.h>
#include <Rendering/Geometry/BoundingSphere.h>
#include <Rendering/Geometry/bbox.h>
#include <Rendering/Geometry/bvh.h>
#include <Rendering/Resources/IMesh.h>

namespace Rendering::Resources
{

	class Mesh : public IMesh
	{
	public:
		Mesh(
			std::span<const Geometry::Vertex> p_vertices,
			std::span<const uint32_t> p_indices,
			uint32_t p_materialIndex = 0
		);
		~Mesh();
		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual uint32_t GetVertexCount() const override;
		virtual uint32_t GetIndexCount() const override;
		virtual const Rendering::Geometry::BoundingSphere& GetBoundingSphere() const override;
		virtual const Rendering::Geometry::bbox& GetBoundingBox()const override { return m_boundingBox; }
		std::vector<uint32_t> GetMaterialIndex() const;
		void AddMaterial(int materialIndex);
		HAL::VertexArray& getVertexArray();
		HAL::VertexBuffer& getVertexBuffer();
		Maths::FVector3 GetVertexPosition(int index);
		Geometry::VertexBVH GetVertexBVH(int index);
		std::vector<Geometry::VertexBVH>& GetVerticesBVH();
		std::vector<uint32_t>& GetIndices();
		void BuildBvh();
		Geometry::Bvh* GetBvh();
	private:
		void Upload(std::span<const Geometry::Vertex> p_vertices, std::span<const uint32_t> p_indices);
		void ComputeBoundingSphereAndBox(std::span<const Geometry::Vertex> p_vertices);

	private:
		const uint32_t m_vertexCount;
		const uint32_t m_indicesCount;
		std::vector<uint32_t>m_materialIndex;
		
		HAL::VertexArray m_vertexArray;
		HAL::VertexBuffer m_vertexBuffer;
		HAL::IndexBuffer m_indexBuffer;
		bool isIndex = false;
		std::vector<Geometry::VertexBVH> m_vertices;
		std::vector<uint32_t> m_indices;
		Geometry::Bvh* m_bvh=nullptr;
		Geometry::bbox m_boundingBox;
		Geometry::BoundingSphere m_boundingSphere;
	};
}