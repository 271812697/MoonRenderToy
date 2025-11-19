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
	/**
	* Standard mesh of OvRendering
	*/
	class Mesh : public IMesh
	{
	public:
		/**
		* Create a mesh with the given vertices, indices and material index
		* @param p_vertices
		* @param p_indices
		* @param p_materialIndex
		*/
		Mesh(
			std::span<const Geometry::Vertex> p_vertices,
			std::span<const uint32_t> p_indices,
			uint32_t p_materialIndex = 0
		);
		~Mesh();

		/**
		* Bind the mesh (Actually bind its VAO)
		*/
		virtual void Bind() const override;

		/**
		* Unbind the mesh (Actually unbind its VAO)
		*/
		virtual void Unbind() const override;

		/**
		* Returns the number of vertices
		*/
		virtual uint32_t GetVertexCount() const override;

		/**
		* Returns the number of indices
		*/
		virtual uint32_t GetIndexCount() const override;

		/**
		* Returns the bounding sphere of the mesh
		*/
		virtual const Rendering::Geometry::BoundingSphere& GetBoundingSphere() const override;

		/**
		* Returns the material index of the mesh
		*/
		std::vector<uint32_t> GetMaterialIndex() const;
		void AddMaterial(int materialIndex);
		HAL::VertexArray& getVertexArray();
		HAL::VertexBuffer& getVertexBuffer();
		void BuildBvh();

	private:
		void Upload(std::span<const Geometry::Vertex> p_vertices, std::span<const uint32_t> p_indices);
		void ComputeBoundingSphere(std::span<const Geometry::Vertex> p_vertices);

	private:
		const uint32_t m_vertexCount;
		const uint32_t m_indicesCount;
		std::vector<uint32_t>m_materialIndex;
		
		HAL::VertexArray m_vertexArray;
		HAL::VertexBuffer m_vertexBuffer;
		HAL::IndexBuffer m_indexBuffer;
		std::vector<Geometry::Vertex> m_vertices;
		std::vector<uint32_t> m_indices;
		Geometry::Bvh* m_bvh;
		Geometry::bbox m_boundingBox;
		Geometry::BoundingSphere m_boundingSphere;
	};
}