#define TINYOBJLOADER_IMPLEMENTATION
#include <iostream>
#include "tinyobjloader/tiny_obj_loader.h"
#include <glad/glad.h>
#include "Mesh.h"
#include "RadeonRays/split_bvh.h"

namespace PathTrace
{
	float sphericalTheta(const Vec3& v)
	{
		return acosf(Math::Clamp(v.y, -1.f, 1.f));
	}

	float sphericalPhi(const Vec3& v)
	{
		float p = atan2f(v.z, v.x);
		return (p < 0.f) ? p + 2.f * PI : p;
	}
	Mesh::Mesh() {
		mBvh = new RadeonRays::SplitBvh(2.0f, 64, 0, 0.001f, 0);
	}
	Mesh::~Mesh() {
		delete mBvh;
		if (hasVAO) {

			glDeleteVertexArrays(1, &vao);
			//glDeleteBuffers(1, &vao);
			glDeleteBuffers(1, &vbop);
			glDeleteBuffers(1, &vbon);
		}


	}
	bool Mesh::LoadFromFile(const std::string& filename)
	{
		mName = filename;
		tinyobj::attrib_t attrib;
		//多个有组织的形状(V,I)
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;
		bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename.c_str(), 0, true);

		if (!ret)
		{
			printf("Unable to load model\n");
			return false;
		}
		//打包成三角形
		// Loop over shapes
		for (size_t s = 0; s < shapes.size(); s++)
		{
			// Loop over faces(polygon)
			size_t index_offset = 0;

			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
			{
				// Loop over vertices in the face.
				for (size_t v = 0; v < 3; v++)
				{
					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
					tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
					tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
					tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
					tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
					tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];

					tinyobj::real_t tx, ty;

					if (!attrib.texcoords.empty())
					{
						tx = attrib.texcoords[2 * idx.texcoord_index + 0];
						ty = 1.0 - attrib.texcoords[2 * idx.texcoord_index + 1];
					}
					else
					{
						if (v == 0)
							tx = ty = 0;
						else if (v == 1)
							tx = 0, ty = 1;
						else
							tx = ty = 1;
					}

					verticesUVX.push_back(Vec4(vx, vy, vz, tx));
					normalsUVY.push_back(Vec4(nx, ny, nz, ty));
				}

				index_offset += 3;
			}
		}
		GenVAO();
		return true;
	}
	void Mesh::Draw() {
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, verticesUVX.size());
		glBindVertexArray(0);
	}

	std::vector<float> Mesh::PackData()
	{
		std::vector<float> res(verticesUVX.size() * 8);
		for (int i = 0; i < verticesUVX.size(); i++) {
			res[8 * i] = verticesUVX[i].x;
			res[8 * i + 1] = verticesUVX[i].y;
			res[8 * i + 2] = verticesUVX[i].z;
			res[8 * i + 3] = normalsUVY[i].x;
			res[8 * i + 4] = normalsUVY[i].y;
			res[8 * i + 5] = normalsUVY[i].z;
			res[8 * i + 6] = verticesUVX[i].w;
			res[8 * i + 7] = normalsUVY[i].w;
		}
		return res;
	}

	RadeonRays::Bvh* Mesh::getBvH()
	{
		return mBvh;
	}

	std::vector<Vec4>& Mesh::getPosUvX()
	{
		return verticesUVX;
	}

	std::vector<Vec4>& Mesh::getNorUvY()
	{
		return normalsUVY;
	}

	std::string Mesh::getName()
	{
		return mName;
	}

	void Mesh::setName(const std::string& name)
	{
		mName = name;
	}

	RadeonRays::bbox& Mesh::getBBox()
	{
		return meshBounds;
	}

	void Mesh::GenVAO()
	{

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbop);
		glGenBuffers(1, &vbon);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbop);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vec4) * verticesUVX.size(), verticesUVX.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);

		glBindBuffer(GL_ARRAY_BUFFER, vbon);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vec4) * normalsUVY.size(), normalsUVY.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
		glBindVertexArray(0);
		hasVAO = true;


	}

	void Mesh::BuildBVH()
	{
		const int numTris = verticesUVX.size() / 3;
		//为所有的三角形构建包围盒，然后在对所有的包围盒构建bvh
		std::vector<RadeonRays::bbox> bounds(numTris);

		for (int i = 0; i < numTris; ++i)
		{
			const Vec3 v1 = Vec3(verticesUVX[i * 3 + 0]);
			const Vec3 v2 = Vec3(verticesUVX[i * 3 + 1]);
			const Vec3 v3 = Vec3(verticesUVX[i * 3 + 2]);

			bounds[i].grow(v1);
			bounds[i].grow(v2);
			bounds[i].grow(v3);
		}

		mBvh->Build(&bounds[0], numTris);
		meshBounds = mBvh->Bounds();
		//mBBox = { mBvh->Bounds().pmin, mBvh->Bounds().pmax };
	}
}