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
#include <Rendering/Settings/EPrimitiveMode.h>

namespace Rendering::Resources
{

	class Mesh : public IMesh
	{
	public:
		Mesh(
			const std::vector<Geometry::Vertex>& p_vertices,
			const std::vector<uint32_t>& p_indices,
			uint32_t p_materialIndex = 0,
			::Rendering::Settings::EPrimitiveMode primitiveMode = Settings::EPrimitiveMode::TRIANGLES
		);
		Mesh(
			const std::vector<Geometry::VertexBVH>& p_vertices,
			const std::vector<uint32_t>& p_indices,
			uint32_t p_materialIndex = 0,
			::Rendering::Settings::EPrimitiveMode primitiveMode = Settings::EPrimitiveMode::TRIANGLES
		);
		~Mesh();
		virtual void Bind(int index=0) const override;
		virtual void Unbind() const override;
		virtual uint32_t GetVertexCount() const override;
		virtual uint32_t GetIndexCount(int index=0) const override;

		virtual const Rendering::Geometry::BoundingSphere& GetBoundingSphere() const override;
		virtual const Rendering::Geometry::bbox& GetBoundingBox()const override { return m_boundingBox; }
		std::vector<uint32_t> GetMaterialIndex() const;
		std::vector<uint32_t> GetSubRangeBufferIndex() const;
		void AddMaterial(int materialIndex,int subRangeIndex);
		HAL::VertexArray& getVertexArray(int index=0);
		HAL::VertexBuffer& getVertexBuffer();
		Maths::FVector3 GetVertexPosition(int index);
		Geometry::VertexBVH GetVertexBVH(int index);
		std::vector<Geometry::VertexBVH>& GetVerticesBVH();
		std::vector<uint32_t>& GetIndices();
		Settings::EPrimitiveMode GetPrimitiveMode() const;
		void SetPrimitiveMode(Settings::EPrimitiveMode mode) { mPrimitiveMode = mode; }
		void BuildBvh();
		Geometry::Bvh* GetBvh();
		void UploadIndices(const std::vector<std::pair<int,int>>&childListt,int index=0);
		void UploadIndices(const std::vector<uint32_t>& p_indices, int index = 0);
		void AddSubRangeBuffer();
		void ComputeBoundingSphereAndBox();
	private:
		void Upload(const std::vector<Geometry::Vertex>& p_vertices, const std::vector<uint32_t>& p_indices);
		void ComputeBoundingSphereAndBox(const std::vector<Geometry::Vertex>& p_vertices);
		void Upload(const std::vector<Geometry::VertexBVH>& p_vertices, const std::vector<uint32_t>& p_indices);
		void ComputeBoundingSphereAndBox(const std::vector<Geometry::VertexBVH>& p_vertices);

	private:
		::Rendering::Settings::EPrimitiveMode mPrimitiveMode = ::Rendering::Settings::EPrimitiveMode::TRIANGLES;
		const uint32_t m_vertexCount;
		const uint32_t m_indicesCount;
		std::vector<uint32_t> uploadIndicesCount;

		//to sure which subRange to which material
		std::vector<uint32_t> m_materialIndex;
		std::vector<uint32_t> m_subRangeIndex;
		
		HAL::VertexBuffer m_vertexBuffer;
		std::vector<std::unique_ptr<HAL::IndexBuffer>> m_IndexBuffers;
		std::vector<std::unique_ptr<HAL::VertexArray>> m_vertexArrays;
		bool isIndex = false;
		std::vector<Geometry::VertexBVH> m_vertices;
		std::vector<uint32_t> m_indices;
		Geometry::Bvh* m_bvh=nullptr;
		Geometry::bbox m_boundingBox;
		Geometry::BoundingSphere m_boundingSphere;
	};
}