#include <iostream>
#include <vector>
#include <fstream>
#include "stb_image/stb_image_resize.h"
#include "stb_image/stb_image.h"
#include "Scene.h"
#include "RadeonRays/bvh_translator.h"
#include "Material.h"
#include "Mesh.h"
#include "Camera.h"
#include "Texture.h"
#include "Renderer.h"
#include "Trace.h"
#include "core/log.h"
namespace PathTrace
{
	Scene::Scene() : mCamera(nullptr), envMap(nullptr), initialized(false), dirty(true) {
		sceneBvh = new RadeonRays::Bvh(10.0f, 64, false);
	}
	Scene::~Scene()
	{
		for (int i = 0; i < meshes.size(); i++)
			delete meshes[i];
		meshes.clear();

		for (int i = 0; i < textures.size(); i++)
			delete textures[i];
		textures.clear();

		if (mCamera)
			delete mCamera;

		if (sceneBvh)
			delete sceneBvh;

		if (envMap)
			delete envMap;
	};

	void Scene::AddCamera(Vec3 pos, Vec3 lookAt, float fov)
	{
		delete mCamera;
		mCamera = new Camera(pos, lookAt, fov);
	}

	int Scene::AddMesh(const std::string& filename)
	{
		int id = -1;
		// Check if mesh was already loaded
		for (int i = 0; i < meshes.size(); i++)
			if (meshes[i]->getName() == filename)
				return i;

		id = meshes.size();
		Mesh* mesh = new Mesh;

		printf("Loading model %s\n", filename.c_str());
		if (mesh->LoadFromFile(filename))
			meshes.push_back(mesh);
		else
		{
			printf("Unable to load model %s\n", filename.c_str());
			delete mesh;
			id = -1;
		}
		return id;
	}

	int Scene::AddTexture(const std::string& filename)
	{
		int id = -1;
		// Check if texture was already loaded
		for (int i = 0; i < textures.size(); i++)
			if (textures[i]->name == filename)
				return i;

		id = textures.size();
		Texture* texture = new Texture;

		printf("Loading texture %s\n", filename.c_str());
		if (texture->LoadTexture(filename))
			textures.push_back(texture);
		else
		{
			printf("Unable to load texture %s\n", filename.c_str());
			delete texture;
			id = -1;
		}

		return id;
	}

	int Scene::AddMaterial(const Material& material)
	{
		int id = materials.size();
		materials.push_back(material);
		return id;
	}

	void Scene::AddEnvMap(const std::string& filename)
	{
		if (envMap)
			delete envMap;

		envMap = new EnvironmentMap;
		if (envMap->LoadMap(filename.c_str())) {
			CORE_INFO("HDR {0} loaded\n", filename.c_str());
		}
		else
		{
			CORE_ERROR("Unable to load HDR");
			delete envMap;
			envMap = nullptr;
		}
		envMapModified = true;
		dirty = true;
	}

	bool Scene::IntersectionByScreen(float x, float y, Vec3& p)
	{
		Vec2 dd = { 2 * x - 1.0f,2 * y - 1.0f };
		float scale = tan(mCamera->getFov() * 0.5);
		//fov水平方向的张角
		dd.y *= renderOptions.renderResolution.y * 1.0f / renderOptions.renderResolution.x * scale;
		dd.x *= scale;
		Vec3 RayDir = mCamera->getRight() * dd.x + mCamera->getUp() * dd.y + mCamera->getForward();
		RayDir = Vec3::Normalize(RayDir);
		Vec3 RayPos = mCamera->getEye();
		Ray r = { RayPos ,RayDir };
		float t = 9999.0f;
		float d = 0.0f;

		// Intersect BVH and tris
		int stack[64];
		int ptr = 0;
		stack[ptr++] = -1;

		int index = bvhTranslator.topLevelIndex;
		float leftHit = 0.0;
		float rightHit = 0.0;

		int currMatID = 0;
		bool BLAS = false;

		Indices triID = { -1,-1,-1 };
		Mat4 transMat;
		int instanceId = -1;
		int transInstanceId = 0;
		Vec3 bary;
		Vec4 vert0, vert1, vert2;

		Ray rTrans;
		rTrans.origin = r.origin;
		rTrans.direction = r.direction;
		while (index != -1)
		{
			Vec3 LRLeaf = bvhTranslator.nodes[index].LRLeaf;
			int leftIndex = int(LRLeaf.x);
			int rightIndex = int(LRLeaf.y);
			int leaf = int(LRLeaf.z);
			if (leaf > 0) // Leaf node of BLAS
			{
				for (int i = 0; i < rightIndex; i++) // Loop through tris
				{

					Vec4 v0 = verticesUVX[vertIndices[leftIndex + i].x];
					Vec4 v1 = verticesUVX[vertIndices[leftIndex + i].y];
					Vec4 v2 = verticesUVX[vertIndices[leftIndex + i].z];

					Vec3 e0 = { v1.x - v0.x, v1.y - v0.y, v1.z - v0.z };
					Vec3 e1 = { v2.x - v0.x,v2.y - v0.y,v2.z - v0.z };
					Vec3 pv = Vec3::Cross(rTrans.direction, e1);
					float det = Vec3::Dot(e0, pv);

					Vec3 tv = { rTrans.origin.x - v0.x ,rTrans.origin.y - v0.y ,rTrans.origin.z - v0.z };
					Vec3 qv = Vec3::Cross(tv, e0);

					Vec4 uvt;
					uvt.x = Vec3::Dot(tv, pv);
					uvt.y = Vec3::Dot(rTrans.direction, qv);
					uvt.z = Vec3::Dot(e1, qv);
					uvt.x = uvt.x / det;
					uvt.y = uvt.y / det;
					uvt.z = uvt.z / det;
					uvt.w = 1.0 - uvt.x - uvt.y;

					if (uvt.x >= 0 && uvt.y >= 0 && uvt.z >= 0 && uvt.w >= 0 && uvt.z < t)
					{
						t = uvt.z;
						triID = vertIndices[leftIndex + i];

						bary = { uvt.w ,uvt.x,uvt.y };
						vert0 = v0, vert1 = v1, vert2 = v2;

						instanceId = transInstanceId;
					}
				}
			}
			else if (leaf < 0) // Leaf node of TLAS
			{
				transInstanceId = -leaf - 1;
				transMat = transforms[-leaf - 1];
				Mat4 inver = transMat.Inverse();

				rTrans.origin = inver.MulPoint(r.origin);
				rTrans.direction = inver.MulDir(r.direction);

				// Add a marker. We'll return to this spot after we've traversed the entire BLAS
				stack[ptr++] = -1;
				index = leftIndex;
				BLAS = true;
				currMatID = rightIndex;
				continue;
			}
			else
			{

				leftHit = AABBIntersect(bvhTranslator.nodes[leftIndex].bboxmax, bvhTranslator.nodes[leftIndex].bboxmin, rTrans.origin, rTrans.direction);
				rightHit = AABBIntersect(bvhTranslator.nodes[rightIndex].bboxmax, bvhTranslator.nodes[rightIndex].bboxmin, rTrans.origin, rTrans.direction);

				if (leftHit > 0.0 && rightHit > 0.0)
				{
					int deferred = -1;
					if (leftHit > rightHit)
					{
						index = rightIndex;
						deferred = leftIndex;
					}
					else
					{
						index = leftIndex;
						deferred = rightIndex;
					}

					stack[ptr++] = deferred;
					continue;
				}
				else if (leftHit > 0.)
				{
					index = leftIndex;
					continue;
				}
				else if (rightHit > 0.)
				{
					index = rightIndex;
					continue;
				}
			}
			index = stack[--ptr];
			// If we've traversed the entire BLAS then switch to back to TLAS and resume where we left off
			if (BLAS && index == -1)
			{
				BLAS = false;

				index = stack[--ptr];

				rTrans.origin = r.origin;
				rTrans.direction = r.direction;
			}
		}
		if (instanceId != -1) {

			Vec3 pos = (vert0 * bary.x + vert1 * bary.y + vert2 * bary.z).xyz();
			p = meshInstances[instanceId].transform.MulPoint(pos);
			selectMeshInstance = instanceId;
			CORE_INFO("Hit {0} pos ({1},{2},{3})", meshInstances[instanceId].name, p.x, p.y, p.z);
			return true;
		}
		return false;
	}

	Vec3 Scene::PathTrace(Vec3 origin, Vec3 direction)
	{
		return Vec3();
	}

	Camera* Scene::getCamera()
	{
		return mCamera;
	}

	std::vector<MeshInstance>& Scene::getMeshInstances()
	{
		return meshInstances;
	}

	MeshInstance Scene::getMeshInstance(int id)
	{
		return meshInstances[id];
	}

	std::vector<std::vector<int>>& Scene::getMeshInstancesTree()
	{
		return meshInstancesTree;
	}

	std::vector<int>& Scene::getMeshInstancesRoots()
	{
		return meshInstancesRoots;
	}

	std::vector<Light>& Scene::getLights()
	{
		return lights;
	}

	RadeonRays::BvhTranslator& Scene::getBvhTranslator()
	{
		return bvhTranslator;
	}

	std::vector<Indices>& Scene::getVertIndices()
	{
		return vertIndices;
	}

	std::vector<Vec4>& Scene::getVerticesUVX()
	{
		return verticesUVX;
	}

	std::vector<Vec4>& Scene::getNormalsUVY()
	{
		return normalsUVY;
	}

	std::vector<Mat4>& Scene::getTransforms()
	{
		return transforms;
	}

	std::vector<Texture*>& Scene::getTextures()
	{
		return textures;
	}

	std::vector<Material>& Scene::getMaterials()
	{
		return materials;
	}

	std::vector<Mesh*>& Scene::getMeshes()
	{
		return meshes;
	}

	RenderOptions& Scene::getRenderOptions()
	{
		return renderOptions;
	}

	EnvironmentMap* Scene::getEnvironmentMap()
	{
		return envMap;
	}

	RadeonRays::bbox& Scene::getBBox()
	{
		return sceneBounds;
	}

	RadeonRays::bbox Scene::getMeshInstanceBox(int id)
	{
		RadeonRays::bbox ret;
		auto meshBox = meshes[meshInstances[id].meshID]->getBvH()->Bounds();

		ret.grow(meshInstances[id].transform.MulPoint(meshBox.pmin));
		ret.grow(meshInstances[id].transform.MulPoint(meshBox.pmax));
		return ret;
	}

	int Scene::getSelectInstanceId()
	{
		return selectMeshInstance;
	}

	void Scene::setPath(const std::string& p)
	{
		path = p;
	}

	void Scene::setDirty(bool flag)
	{
		dirty = flag;
	}

	int Scene::AddMeshInstance(const MeshInstance& meshInstance)
	{
		int id = meshInstances.size();
		meshInstances.push_back(meshInstance);
		return id;
	}

	int Scene::AddLight(const Light& light)
	{
		int id = lights.size();
		lights.push_back(light);
		return id;
	}
	// Loop through all the mesh Instances and build a Top Level BVH
	void Scene::createTLAS()
	{
		for (int i = 0; i < meshInstances.size(); i++) {
			Mat4 T = meshInstances[i].localform;
			int id = meshInstances[i].parentID;
			if (id == -1) {
				meshInstances[i].transform = meshInstances[i].localform;
			}
			else
			{
				//it is right?
				while (id != -1)
				{
					T = T * meshInstances[id].localform;
					id = meshInstances[id].parentID;
				}
				meshInstances[i].transform = T;
			}

		}
		std::vector<RadeonRays::bbox> bounds;
		bounds.resize(meshInstances.size());
		//遍历所有的MeshInstance,将mesh做变换后的包围盒构建顶层BVH
		for (int i = 0; i < meshInstances.size(); i++)
		{
			RadeonRays::bbox bbox = meshes[meshInstances[i].meshID]->getBvH()->Bounds();
			Mat4 matrix = meshInstances[i].transform;

			Vec3 minBound = bbox.pmin;
			Vec3 maxBound = bbox.pmax;

			Vec3 right = Vec3(matrix[0][0], matrix[0][1], matrix[0][2]);
			Vec3 up = Vec3(matrix[1][0], matrix[1][1], matrix[1][2]);
			Vec3 forward = Vec3(matrix[2][0], matrix[2][1], matrix[2][2]);
			Vec3 translation = Vec3(matrix[3][0], matrix[3][1], matrix[3][2]);

			Vec3 xa = right * minBound.x;
			Vec3 xb = right * maxBound.x;

			Vec3 ya = up * minBound.y;
			Vec3 yb = up * maxBound.y;

			Vec3 za = forward * minBound.z;
			Vec3 zb = forward * maxBound.z;

			minBound = Vec3::Min(xa, xb) + Vec3::Min(ya, yb) + Vec3::Min(za, zb) + translation;
			maxBound = Vec3::Max(xa, xb) + Vec3::Max(ya, yb) + Vec3::Max(za, zb) + translation;

			RadeonRays::bbox bound;
			bound.pmin = minBound;
			bound.pmax = maxBound;

			bounds[i] = bound;
		}
		sceneBvh->Build(&bounds[0], bounds.size());
		sceneBounds = sceneBvh->Bounds();
	}
	//遍历meshes，每一个meshg构建BVH
	void Scene::createBLAS()
	{
		for (int i = 0; i < meshes.size(); i++)
		{
			CORE_INFO("Building BVH for {0}\n", meshes[i]->getName().c_str());
			meshes[i]->BuildBVH();
		}
	}

	void Scene::RebuildInstances()
	{
		delete sceneBvh;
		sceneBvh = new RadeonRays::Bvh(10.0f, 64, false);
		createTLAS();
		bvhTranslator.UpdateTLAS(sceneBvh, meshInstances);
		for (int i = 0; i < meshInstances.size(); i++)
			transforms[i] = meshInstances[i].transform;
		instancesModified = true;
		dirty = true;
	}

	void Scene::Save()
	{
		//to do:
	}

	void Scene::ProcessScene()
	{
		createBLAS();
		meshInstancesTree.resize(meshInstances.size());
		for (int i = 0; i < meshInstances.size(); i++) {
			if (meshInstances[i].parentID != -1) {
				meshInstancesTree[meshInstances[i].parentID].push_back(i);
			}
			else
			{
				meshInstancesRoots.push_back(i);
			}
		}
		createTLAS();
		bvhTranslator.Process(sceneBvh, meshes, meshInstances);
		// Copy mesh data
		int verticesCnt = 0;
		CORE_INFO("Copying Mesh Data\n");
		for (int i = 0; i < meshes.size(); i++)
		{
			// Copy indices from BVH and not from Mesh. 
			// Required if splitBVH is used as a triangle can be shared by leaf nodes
			int numIndices = meshes[i]->getBvH()->GetNumIndices();
			const int* triIndices = meshes[i]->getBvH()->GetIndices();
			for (int j = 0; j < numIndices; j++)
			{
				int index = triIndices[j];
				int v1 = (index * 3 + 0) + verticesCnt;
				int v2 = (index * 3 + 1) + verticesCnt;
				int v3 = (index * 3 + 2) + verticesCnt;
				vertIndices.push_back(Indices{ v1, v2, v3 });
			}
			verticesUVX.insert(verticesUVX.end(), meshes[i]->getPosUvX().begin(), meshes[i]->getPosUvX().end());
			normalsUVY.insert(normalsUVY.end(), meshes[i]->getNorUvY().begin(), meshes[i]->getNorUvY().end());
			verticesCnt += meshes[i]->getPosUvX().size();
		}
		// Copy transforms
		CORE_INFO("Copying transforms\n");
		transforms.resize(meshInstances.size());
		for (int i = 0; i < meshInstances.size(); i++)
			transforms[i] = meshInstances[i].transform;
		// Copy textures
		if (!textures.empty())
			CORE_INFO("Copying and resizing textures\n");
		int reqWidth = renderOptions.texArrayWidth;
		int reqHeight = renderOptions.texArrayHeight;
		int texBytes = reqWidth * reqHeight * 4;
		textureMapsArray.resize(texBytes * textures.size());
		for (int i = 0; i < textures.size(); i++)
		{
			int texWidth = textures[i]->width;
			int texHeight = textures[i]->height;
			// Resize textures to fit 2D texture array
			if (texWidth != reqWidth || texHeight != reqHeight)
			{
				unsigned char* resizedTex = new unsigned char[texBytes];
				stbir_resize_uint8(&textures[i]->texData[0], texWidth, texHeight, 0, resizedTex, reqWidth, reqHeight, 0, 4);
				std::copy(resizedTex, resizedTex + texBytes, &textureMapsArray[i * texBytes]);
				delete[] resizedTex;
			}
			else
				std::copy(textures[i]->texData.begin(), textures[i]->texData.end(), &textureMapsArray[i * texBytes]);
		}
		// Add a default camera
		if (!mCamera)
		{
			RadeonRays::bbox bounds = sceneBvh->Bounds();
			Vec3 extents = bounds.extents();
			Vec3 center = bounds.center();
			AddCamera(Vec3(center.x, center.y, center.z + Vec3::Length(extents) * 2.0f), center, 45.0f);
		}
		initialized = true;
	}
}