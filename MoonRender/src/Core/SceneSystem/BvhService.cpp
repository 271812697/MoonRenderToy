#include <algorithm>
#include <string>
#include <tinyxml2.h>
#include <tracy/Tracy.hpp>
#include <Rendering/HAL/TextureHandle.h>
#include <Rendering/Resources/Texture.h>
#include <Core/ECS/Components/CAmbientSphereLight.h>
#include <Core/ECS/Components/CDirectionalLight.h>
#include <Core/ECS/Components/CMaterialRenderer.h>
#include <Core/Global/ServiceLocator.h>
#include <Core/ResourceManagement/MaterialManager.h>
#include <Core/ResourceManagement/ModelManager.h>
#include <Core/SceneSystem/Scene.h>
#include <Core/SceneSystem/BvhService.h>
#include "stb_image/stb_image_resize.h"
#include <fstream>
namespace Core::SceneSystem
{
	bool exportObjOnlyVertices(
		const std::string& filename,
		const float* vertices,
		const unsigned int* indices,
		size_t vertexCount,
		size_t indexCount
	) {
		// 前置校验
		if (vertices == nullptr || indices == nullptr) {
			std::cerr << "错误：顶点/索引数组为空！" << std::endl;
			return false;
		}
		if (vertexCount == 0 || indexCount == 0) {
			std::cerr << "错误：顶点/索引数量不能为0！" << std::endl;
			return false;
		}
		if (indexCount % 3 != 0) {
			std::cerr << "错误：索引数量必须是3的倍数（三角面）！当前数量：" << indexCount << std::endl;
			return false;
		}

		try {
			// 打开文件（覆盖写入）
			std::ofstream objFile(filename);
			if (!objFile.is_open()) {
				throw std::runtime_error("无法打开文件：" + filename);
			}

			// 写入文件头（可选，方便查看信息）
			objFile << "# 自动生成的OBJ模型（仅顶点坐标）\n";
			objFile << "# 顶点数：" << vertexCount << " | 三角面数：" << (indexCount / 3) << "\n\n";

			// 第一步：写入顶点坐标（OBJ格式：v x y z）
			for (size_t i = 0; i < vertexCount; ++i) {
				size_t offset = i * 3; // 每个顶点占3个float（x,y,z）
				objFile << "v "
					<< vertices[offset] << " "    // x
					<< vertices[offset + 1] << " "// y
					<< vertices[offset + 2] << "\n";// z
			}
			objFile << "\n"; // 空行分隔，提升可读性

			// 第二步：写入三角面索引（OBJ索引从1开始，需+1转换）
			for (size_t i = 0; i < indexCount; i += 3) {
				objFile << "f "
					<< (indices[i] + 1) << " "    // 第一个顶点索引
					<< (indices[i + 1] + 1) << " "// 第二个顶点索引
					<< (indices[i + 2] + 1) << "\n";// 第三个顶点索引
			}

			// 关闭文件
			objFile.close();
			std::cout << "✅ OBJ文件导出成功！路径：" << filename << std::endl;
			return true;
		}
		catch (const std::exception& e) {
			std::cerr << "❌ 导出失败：" << e.what() << std::endl;
			return false;
		}
	}
	BvhService::BvhService(Scene* sc):scene(sc)
	{
	}
	void BvhService::AddMaterial(::Core::Resources::Material* material)
	{
		Material tempMat;
		if (material) {
			auto& propsMap=material->GetProperties();
			auto it = propsMap.find("u_Albedo");
			if (it != propsMap.end()) {
				tempMat.baseColor=std::get<Maths::FVector4>(it->second.value).ToFVector3();
				tempMat.opacity = std::get<Maths::FVector4>(it->second.value).w;
			
			}
			it = propsMap.find("u_EmissiveColor");
			if (it != propsMap.end()) {
				tempMat.emission = std::get<Maths::FVector3>(it->second.value);
			}
			it = propsMap.find("u_Metallic");
			if (it != propsMap.end()) {
				tempMat.metallic= std::get<float>(it->second.value);
			}
			it = propsMap.find("u_Roughness");
			if (it != propsMap.end()) {
				tempMat.roughness = std::get<float>(it->second.value);
			}
			//u_RefractionIndex
			it = propsMap.find("u_RefractionIndex");
			if (it != propsMap.end()) {
				tempMat.ior = std::get<float>(it->second.value);
			}

			//Albedo Texture
			it = propsMap.find("u_AlbedoMap");
			if (it!=propsMap.end()) {
				::Rendering::HAL::Texture* handle = nullptr;
				if (auto textureHandle = std::get_if<::Rendering::HAL::TextureHandle*>(&it->second.value))
				{
					handle = static_cast<::Rendering::HAL::Texture*>(*textureHandle);
				}
				else if (auto texture = std::get_if<::Rendering::Resources::Texture*>(&it->second.value))
				{
					if (*texture != nullptr)
					{
						handle = &(*texture)->GetTexture();					
					}
				}
				if (handle) {
					tempMat.baseColorTexId=AddTexture(handle);
				}
			}
			it = propsMap.find("u_RoughnessMap");
			if (it != propsMap.end()) {
				::Rendering::HAL::Texture* handle = nullptr;
				if (auto textureHandle = std::get_if<::Rendering::HAL::TextureHandle*>(&it->second.value))
				{
					handle = static_cast<::Rendering::HAL::Texture*>(*textureHandle);
				}
				else if (auto texture = std::get_if<::Rendering::Resources::Texture*>(&it->second.value))
				{
					if (*texture != nullptr)
					{
						handle = &(*texture)->GetTexture();
					}
				}
				if (handle) {
					tempMat.metallicRoughnessTexID = AddTexture(handle);
				}
			}
			it = propsMap.find("u_EmissiveMap");
			if (it != propsMap.end()) {
				::Rendering::HAL::Texture* handle = nullptr;
				if (auto textureHandle = std::get_if<::Rendering::HAL::TextureHandle*>(&it->second.value))
				{
					handle = static_cast<::Rendering::HAL::Texture*>(*textureHandle);
				}
				else if (auto texture = std::get_if<::Rendering::Resources::Texture*>(&it->second.value))
				{
					if (*texture != nullptr)
					{
						handle = &(*texture)->GetTexture();
					}
				}
				if (handle) {
					tempMat.emissionmapTexID = AddTexture(handle);
				}
			}
			it = propsMap.find("u_NormalMap");
			if (it != propsMap.end()) {
				::Rendering::HAL::Texture* handle = nullptr;
				if (auto textureHandle = std::get_if<::Rendering::HAL::TextureHandle*>(&it->second.value))
				{
					handle = static_cast<::Rendering::HAL::Texture*>(*textureHandle);
				}
				else if (auto texture = std::get_if<::Rendering::Resources::Texture*>(&it->second.value))
				{
					if (*texture != nullptr)
					{
						handle = &(*texture)->GetTexture();
					}
				}
				if (handle) {
					tempMat.normalmapTexID= AddTexture(handle);
				}
			}		
		}
		materials.push_back(tempMat);
	}
	int	 BvhService::AddTexture(::Rendering::HAL::Texture* tex)
	{
		int id = -1;
		// Check if texture was already loaded
		for (int i = 0; i < textures.size(); i++)
			if (textures[i] == tex)
				return i;
		id = textures.size();
		textures.push_back(tex);
		return id;
	}
	void BvhService::ProcessBLAS() {
			int nodeCnt = 0;
			for (int i = 0; i < meshes.size(); i++)
				nodeCnt += meshes[i]->GetBvh()->m_nodecnt;
			topLevelIndex = nodeCnt;
			// reserve space for top level nodes
			nodeCnt += 2 * meshInstances.size();
			nodes.resize(nodeCnt);

			int bvhRootIndex = 0;
			curTriIndex = 0;

			for (int i = 0; i < meshes.size(); i++)
			{
				::Rendering::Resources::Mesh* mesh = meshes[i];
				curNode = bvhRootIndex;

				bvhRootStartIndices.push_back(bvhRootIndex);
				bvhRootIndex += mesh->GetBvh()->m_nodecnt;

				ProcessBLASNodes(mesh->GetBvh()->m_root);
				curTriIndex += mesh->GetBvh()->GetNumIndices();
			}

		}
	void BvhService::ProcessTLAS() {
		curNode = topLevelIndex;
		ProcessTLASNodes(topLevelBvh->m_root);
	}
	int BvhService::ProcessBLASNodes(const ::Rendering::Geometry::Bvh::Node* node) {
		::Rendering::Geometry::bbox bbox = node->bounds;

		nodes[curNode].bboxmin = bbox.pmin;
		nodes[curNode].bboxmax = bbox.pmax;
		nodes[curNode].LRLeaf.z = 0;

		int index = curNode;
		//BLAS的叶子节点z=1 x y为二级索引
		if (node->type == ::Rendering::Geometry::Bvh::NodeType::kLeaf)
		{
			nodes[curNode].LRLeaf.x = curTriIndex + node->startidx;
			nodes[curNode].LRLeaf.y = node->numprims;
			nodes[curNode].LRLeaf.z = 1;
		}
		else
		{
			curNode++;
			nodes[index].LRLeaf.x = ProcessBLASNodes(node->lc);
			curNode++;
			nodes[index].LRLeaf.y = ProcessBLASNodes(node->rc);
		}
		return index;
	}
	int BvhService::ProcessTLASNodes(const ::Rendering::Geometry::Bvh::Node* node) {
		::Rendering::Geometry::bbox bbox = node->bounds;

		nodes[curNode].bboxmin = bbox.pmin;
		nodes[curNode].bboxmax = bbox.pmax;
		nodes[curNode].LRLeaf.z = 0;

		int index = curNode;

		if (node->type == ::Rendering::Geometry::Bvh::NodeType::kLeaf)
		{
			//叶子节点
			int instanceIndex = topLevelBvh->m_packed_indices[node->startidx];
			int meshIndex = meshInstances[instanceIndex].meshID;
			int materialID = meshInstances[instanceIndex].materialID;

			nodes[curNode].LRLeaf.x = bvhRootStartIndices[meshIndex];
			nodes[curNode].LRLeaf.y = materialID;
			nodes[curNode].LRLeaf.z = -instanceIndex - 1;
		}
		else
		{
			//内部节点存储左右子节点的索引
			curNode++;
			nodes[index].LRLeaf.x = ProcessTLASNodes(node->lc);
			curNode++;
			nodes[index].LRLeaf.y = ProcessTLASNodes(node->rc);
		}
		return index;
	}
	void BvhService::UpdateTLAS(const ::Rendering::Geometry::Bvh* topLevelBvh, const std::vector<MeshInstance>& instances) {
		this->topLevelBvh = topLevelBvh;
		meshInstances = instances;
		curNode = topLevelIndex;
		ProcessTLASNodes(topLevelBvh->m_root);
	}
	void BvhService::Process(const ::Rendering::Geometry::Bvh* topLevelBvh, const std::vector<::Rendering::Resources::Mesh*>& sceneMeshes, const std::vector<MeshInstance>& instances) {
		if (topLevelBvh->m_root ==nullptr) {
			return;
		}
		this->topLevelBvh = topLevelBvh;
		meshes = sceneMeshes;
		meshInstances = instances;
		ProcessBLAS();
		ProcessTLAS();
			
		//Copy Mesh vertex data to a batch buffer
		vertIndices.clear();
		verticesUVX.clear();
		normalsUVY.clear();
		transforms.clear();
		int verticesCnt = 0;
		for (int i = 0;i < meshes.size();i++) {
			int numIndices = meshes[i]->GetBvh()->GetNumIndices();
			const int* triIndices = meshes[i]->GetBvh()->GetIndices();
			auto& indexArr = meshes[i]->GetIndices();
			bool isIndexMesh = indexArr.size() > 0;
			for (int j = 0; j < numIndices; j++)
			{
				int index = triIndices[j];
				if (isIndexMesh) {	
					int v1 = indexArr[(index * 3 + 0)] + verticesCnt;
					int v2 = indexArr[(index * 3 + 1)] + verticesCnt;
					int v3 = indexArr[(index * 3 + 2)] + verticesCnt;
					vertIndices.push_back(Indices{ v1, v2, v3 });
				}
				else
				{
					int v1 = (index * 3 + 0) + verticesCnt;
					int v2 = (index * 3 + 1) + verticesCnt;
					int v3 = (index * 3 + 2) + verticesCnt;
					vertIndices.push_back(Indices{ v1, v2, v3 });
				}
			}
			auto& vertexData=meshes[i]->GetVerticesBVH();
			for (int k = 0;k < vertexData.size();k++) {
				verticesUVX.push_back(Maths::FVector4(vertexData[k].position.x, vertexData[k].position.y, vertexData[k].position.z, vertexData[k].texCoords.x));
				normalsUVY.push_back(Maths::FVector4(vertexData[k].normals.x, vertexData[k].normals.y, vertexData[k].normals.z, vertexData[k].texCoords.y));
			}
			verticesCnt += vertexData.size();
		}
		// Copy transforms
		transforms.resize(meshInstances.size());
		for (int i = 0; i < meshInstances.size(); i++)
			transforms[i] = Maths::FMatrix4::Transpose(meshInstances[i].transform);
		// Copy Textures
		int reqWidth = renderOptions.texArrayWidth;
		int reqHeight = renderOptions.texArrayHeight;
		int texBytes = reqWidth * reqHeight * 4;
		textureMapsArray.resize(texBytes * textures.size());
		for (int i = 0; i < textures.size(); i++)
		{
			int texWidth = textures[i]->GetWidth();
			int texHeight = textures[i]->GetHeight();
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
		isDirty = true;
		SaveAsObj("res.obj");
	}
	void BvhService::SaveAsObj(const std::string& path)
	{
		std::vector<Maths::FVector3>vertices;
		std::vector<unsigned int>indices;
		for (int i = 0; i <meshInstances.size(); i++) {
			int meshId =meshInstances[i].meshID;
			auto matrix = meshInstances[i].transform;
			auto& mesh =meshes[meshId];
			int numTrs = mesh->GetIndexCount() ? mesh->GetIndexCount() / 3 : mesh->GetVertexCount() / 3;
			for (int triIndex = 0; triIndex < numTrs; triIndex++) {
				Maths::FVector3 v0 = mesh->GetVertexPosition(triIndex * 3);
				Maths::FVector3 v1 = mesh->GetVertexPosition(triIndex * 3 + 1);
				Maths::FVector3 v2 = mesh->GetVertexPosition(triIndex * 3 + 2);
				vertices.push_back(Maths::FMatrix4::MulPoint(matrix, v0));
				vertices.push_back(Maths::FMatrix4::MulPoint(matrix, v1));
				vertices.push_back(Maths::FMatrix4::MulPoint(matrix, v2));
			}
		}
		indices.resize(vertices.size());
		for (int i = 0; i < indices.size(); i++) {
			indices[i] = i;
		}
		//exportObjOnlyVertices(path,(float*)vertices.data(),indices.data(),vertices.size(),indices.size());
	}
	void BvhService::Clear() {
		if (m_sceneBvh) {
			delete m_sceneBvh;
		}
		m_sceneBvh = new ::Rendering::Geometry::Bvh(10.0f, 64, false);
		meshInstances.clear();
		meshes.clear();
		nodes.clear();
		bvhRootStartIndices.clear();
		materials.clear();
		lights.clear();
		topLevelBvh = nullptr;
		transforms.clear();
		textures.clear();
		textureMapsArray.clear();
		verticesUVX.clear();
		normalsUVY.clear();
		vertIndices.clear();
		triangleInfoMap.clear();
		curNode = 0;
		curTriIndex = 0;
		topLevelIndex = 0;
		// Collect all model renderers' bounds
			
	}
	bool BvhService::DirtyFlag()
	{
		return isDirty;
	}
	void BvhService::SetDirtyFlag(bool flag)
	{
		isDirty = flag;
	}
	void BvhService::AddTriangleInfo(int instanceId,const TriangleInfo& info)
	{
		for (int i = 0; i < triangleInfoMap[instanceId].size(); i++) {
			if ((triangleInfoMap[instanceId][i].info)== (info.info)) {
				return;
			}
		}
		triangleInfoMap[instanceId].push_back(info);
		isTriangleDirty = true;

		//batch triangleInfo
		int triOffset = 0;
		meshTriangleInfo.clear();
		triangleInfo.clear();
		std::vector<unsigned int>baseOffset(meshes.size(),0);
		meshTriangleInfo.resize(meshes.size());
		for (int i = 0; i < meshTriangleInfo.size(); i++) {
			meshTriangleInfo[i].start = -1;
			meshTriangleInfo[i].num = -1;
			baseOffset[i] = triOffset;
			triOffset+= meshes[i]->GetBvh()->GetNumIndices();
		}
		for (auto& it : triangleInfoMap) {
			if (it.second.size() > 0) {
				int mid = meshInstances[it.first].meshID;
				meshTriangleInfo[mid].start = triangleInfo.size();
				meshTriangleInfo[mid].num = it.second.size();
				triangleInfo.insert(triangleInfo.end(), it.second.begin(), it.second.end());
				meshTriangleInfo[mid].baseOffset = baseOffset[mid];
			}
		}

	}
	void BvhService::UpdateTriangleInfo()
	{
		if (isTriangleDirty) {
			for (auto& it : triangleInfoMap) {
				if (it.second.size() > 0) {
					auto instanceId = it.first;
					auto mid = meshInstances[instanceId].meshID;
					auto actorId = meshInstances[instanceId].actorID;
					auto actor = scene->FindActorByID(actorId);
					if (actor->GetTag() == "Geomerty") {
						auto matList = actor->GetComponent<Core::ECS::Components::CMaterialRenderer>();
						if (matList) {
							auto mat = matList->GetMaterialAtIndex(0);
							if (mat) {
								mat->AddFeature("TRIANGLE_INFO");
								auto& tfo=it.second;
								::Rendering::HAL::GLTexture* triangleInfoTex = nullptr;	
								// TriangleInfo Texture
								::Rendering::Settings::TextureDesc desc;
								desc.isTextureBuffer = true;
								desc.internalFormat = ::Rendering::Settings::EInternalFormat::RG32UI;
								desc.buffetLen = tfo.size() * sizeof(::Core::SceneSystem::TriangleInfo);
								desc.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
									.data = tfo.data()
								};
								int ss = tfo.size();
								mat->SetProperty("trifoLen",ss);
								const ::Rendering::Data::MaterialProperty prop=mat->GetProperty("triangleInfoTex").value();
								if (std::holds_alternative<::Rendering::Resources::Texture*>(prop.value)) {
									
									auto v = std::get<::Rendering::Resources::Texture*>(prop.value);

									if (v == nullptr) {

										triangleInfoTex = new ::Rendering::HAL::GLTexture(::Rendering::Settings::ETextureType::TEXTURE_BUFFER);
										triangleInfoTex->Allocate(desc);
										mat->SetProperty("triangleInfoTex", triangleInfoTex);
									}
									else
									{
										v->GetTexture().Allocate(desc);
									}
								}
								else
								{
									auto v = std::get<::Rendering::HAL::TextureHandle*>(prop.value);
									::Rendering::HAL::GLTexture* ttex = static_cast<::Rendering::HAL::GLTexture*>(v);
									ttex->Allocate(desc);
								}
							}
						}
					}
				}
			}
		}
	}
	BvhService::~BvhService() {
		delete m_sceneBvh;
	}
}
