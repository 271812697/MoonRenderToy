#include <array>
#include <Rendering/Resources/Mesh.h>
#include "Rendering/Geometry/split_bvh.h"

Rendering::Resources::Mesh::Mesh(
	std::span<const Geometry::Vertex> p_vertices,
	std::span<const uint32_t> p_indices,
	uint32_t p_materialIndex
) :
	m_vertexCount(static_cast<uint32_t>(p_vertices.size())),
	m_indicesCount(static_cast<uint32_t>(p_indices.size()))
{
	isIndex = m_indicesCount> 0;
	
	m_indices.resize(m_indicesCount);
	m_vertices.resize(m_vertexCount);
	memcpy(m_indices.data(), p_indices.data(), p_indices.size_bytes());
	for (int i = 0;i < p_vertices.size();i++) {
	
		m_vertices[i].position.x = p_vertices[i].position[0];
		m_vertices[i].position.y = p_vertices[i].position[1];
		m_vertices[i].position.z = p_vertices[i].position[2];

		m_vertices[i].texCoords.x = p_vertices[i].texCoords[0];
		m_vertices[i].texCoords.y= p_vertices[i].texCoords[1];

		m_vertices[i].normals.x = p_vertices[i].normals[0];
		m_vertices[i].normals.y = p_vertices[i].normals[1];
		m_vertices[i].normals.z = p_vertices[i].normals[2];
	}
	m_materialIndex.push_back(p_materialIndex);
	Upload(p_vertices, p_indices);
	ComputeBoundingSphereAndBox(p_vertices);

}
Rendering::Resources::Mesh::~Mesh()
{
	delete m_bvh;
}
	
void Rendering::Resources::Mesh::Bind() const
{
	m_vertexArray.Bind();
}

void Rendering::Resources::Mesh::Unbind() const
{
	m_vertexArray.Unbind();
}

uint32_t Rendering::Resources::Mesh::GetVertexCount() const
{
	return m_vertexCount;
}

uint32_t Rendering::Resources::Mesh::GetIndexCount() const
{
	return m_indicesCount;
}

const Rendering::Geometry::BoundingSphere& Rendering::Resources::Mesh::GetBoundingSphere() const
{
	return m_boundingSphere;
}

std::vector<uint32_t> Rendering::Resources::Mesh::GetMaterialIndex() const
{
	return m_materialIndex;
}

void Rendering::Resources::Mesh::AddMaterial(int materialIndex)
{
	m_materialIndex.push_back(materialIndex);
}

Rendering::HAL::VertexArray& Rendering::Resources::Mesh::getVertexArray()
{
	return m_vertexArray;
}

Rendering::HAL::VertexBuffer& Rendering::Resources::Mesh::getVertexBuffer()
{
	return m_vertexBuffer;
}

Maths::FVector3 Rendering::Resources::Mesh::GetVertexPosition(int index)
{
	return isIndex?m_vertices[m_indices[index]].position: m_vertices[index].position;
}

void Rendering::Resources::Mesh::BuildBvh()
{
	if (m_bvh) {
		delete m_bvh;
		m_bvh = nullptr;
	}
	
	//currently only build for triangles
	
	int numTris = isIndex ? m_indicesCount / 3 : m_vertexCount / 3;
	if (numTris > 0) {
		m_bvh = new Geometry::SplitBvh(2.0f, 64, 0, 0.001f, 0);
		//为所有的三角形构建包围盒，然后在对所有的包围盒构建bvh
		std::vector<Geometry::bbox> bounds(numTris);

		for (int i = 0; i < numTris; ++i)
		{
			if (isIndex) {
				bounds[i].grow(m_vertices[m_indices[3 * i]].position);
				bounds[i].grow(m_vertices[m_indices[3 * i+1]].position);
				bounds[i].grow(m_vertices[m_indices[3 * i+2]].position);
			}
			else
			{			
				bounds[i].grow(m_vertices[3 * i].position);
				bounds[i].grow(m_vertices[3 * i + 1].position);
				bounds[i].grow(m_vertices[3 * i + 2].position);
			}
		}
		m_bvh->Build(&bounds[0], numTris);
	}
}

Rendering::Geometry::Bvh* Rendering::Resources::Mesh::GetBvh()
{
	return m_bvh;
}

void Rendering::Resources::Mesh::Upload(std::span<const Geometry::Vertex> p_vertices, std::span<const uint32_t> p_indices)
{
	if (m_vertexBuffer.Allocate(p_vertices.size_bytes()))
	{
		m_vertexBuffer.Upload(p_vertices.data());

		if (m_indexBuffer.Allocate(p_indices.size_bytes()))
		{
			m_indexBuffer.Upload(p_indices.data());
		}
		else
		{
			("Empty index buffer!");
		}
		m_vertexArray.SetLayout(std::to_array<Settings::VertexAttribute>({
			{ Settings::EDataType::FLOAT, 3 }, // position
			{ Settings::EDataType::FLOAT, 2 }, // texCoords
			{ Settings::EDataType::FLOAT, 3 }, // normal
			{ Settings::EDataType::FLOAT, 3 }, // tangent
			{ Settings::EDataType::FLOAT, 3 }  // bitangent
			}), m_vertexBuffer, m_indexBuffer);
	}
	else
	{
		//("Empty vertex buffer!");
	}
}

void Rendering::Resources::Mesh::ComputeBoundingSphereAndBox(std::span<const Geometry::Vertex> p_vertices)
{
	m_boundingSphere.position = Maths::FVector3::Zero;
	m_boundingSphere.radius = 0.0f;
	

	if (!p_vertices.empty())
	{
		float minX = std::numeric_limits<float>::max();
		float minY = std::numeric_limits<float>::max();
		float minZ = std::numeric_limits<float>::max();

		float maxX = std::numeric_limits<float>::min();
		float maxY = std::numeric_limits<float>::min();
		float maxZ = std::numeric_limits<float>::min();

		for (const auto& vertex : p_vertices)
		{
			minX = std::min(minX, vertex.position[0]);
			minY = std::min(minY, vertex.position[1]);
			minZ = std::min(minZ, vertex.position[2]);

			maxX = std::max(maxX, vertex.position[0]);
			maxY = std::max(maxY, vertex.position[1]);
			maxZ = std::max(maxZ, vertex.position[2]);
		}

		m_boundingSphere.position = Maths::FVector3{ minX + maxX, minY + maxY, minZ + maxZ } / 2.0f;
		m_boundingBox = Geometry::bbox(Maths::FVector3{ minX , minY , minZ  }, Maths::FVector3{  maxX,  maxY, maxZ });
		for (const auto& vertex : p_vertices)
		{
			const auto& position = reinterpret_cast<const Maths::FVector3&>(vertex.position);
			m_boundingSphere.radius = std::max(m_boundingSphere.radius, Maths::FVector3::Distance(m_boundingSphere.position, position));
		}
		BuildBvh();
	}

	
}
