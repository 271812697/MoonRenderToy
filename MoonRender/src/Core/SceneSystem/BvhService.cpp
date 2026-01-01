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
	namespace {
		// 定义2D点结构体，简化坐标操作
		using Point2D = Maths::FVector2;
		// 全局浮点容错阈值（避免浮点计算精度误差导致的误判）
		const float EPS = 1e-7;

		/**
		 * 向量叉乘计算函数
		 * @param A 起点
		 * @param B 第二个点
		 * @param C 第三个点
		 * @return 叉乘结果：(B-A) × (C-A)
		 */
		float cross(const Point2D& A, const Point2D& B, const Point2D& C) {
			return (B.x - A.x) * (C.y - A.y) - (B.y - A.y) * (C.x - A.x);
		}

		/**
		 * 判断点是否在轴对齐矩形（AABB）内部（含边界）
		 * @param P 待判断点
		 * @param rect_xmin 矩形最小x坐标
		 * @param rect_ymin 矩形最小y坐标
		 * @param rect_xmax 矩形最大x坐标
		 * @param rect_ymax 矩形最大y坐标
		 * @return true：点在矩形内/边界上，false：点在矩形外
		 */
		bool isPointInAABB(const Point2D& P,
			float rect_xmin, float rect_ymin,
			float rect_xmax, float rect_ymax) {
			return (P.x >= rect_xmin - EPS) && (P.x <= rect_xmax + EPS) &&
				(P.y >= rect_ymin - EPS) && (P.y <= rect_ymax + EPS);
		}

		/**
		 * 判断点是否在三角形内部（含边界，叉乘法）
		 * @param Q 待判断点
		 * @param P0 三角形顶点1
		 * @param P1 三角形顶点2
		 * @param P2 三角形顶点3
		 * @return true：点在三角形内/边界上，false：点在三角形外
		 */
		bool isPointInTriangle(const Point2D& Q,
			const Point2D& P0, const Point2D& P1, const Point2D& P2) {
			float c1 = cross(P0, P1, Q);
			float c2 = cross(P1, P2, Q);
			float c3 = cross(P2, P0, Q);

			// 三者同号（均≥0 或 均≤0），容错EPS处理边界情况
			bool allNonNeg = (c1 >= -EPS) && (c2 >= -EPS) && (c3 >= -EPS);
			bool allNonPos = (c1 <= EPS) && (c2 <= EPS) && (c3 <= EPS);

			return allNonNeg || allNonPos;
		}

		/**
		 * 判断两条线段是否相交（含端点、含共线重叠，跨立实验）
		 * @param A 线段1端点1
		 * @param B 线段1端点2
		 * @param C 线段2端点1
		 * @param D 线段2端点2
		 * @return true：线段相交/接触/共线重叠，false：线段不相交
		 */
		bool isSegmentIntersect(const Point2D& A, const Point2D& B,
			const Point2D& C, const Point2D& D) {
			// 计算各叉乘结果
			float c1 = cross(A, B, C);
			float c2 = cross(A, B, D);
			float c3 = cross(C, D, A);
			float c4 = cross(C, D, B);

			// 条件1：互相跨立（端点在对方两侧或在对方线段上）
			bool crossCondition = (c1 * c2 <= EPS) && (c3 * c4 <= EPS);

			// 条件2：处理共线情况（判断投影是否重叠）
			bool collinear = (fabs(c1) < EPS) && (fabs(c2) < EPS);
			if (collinear) {
				// 辅助函数：获取线段在x/y轴上的投影范围
				auto getSegmentRange = [](const Point2D& P, const Point2D& Q) {
					std::pair<float, float> xRange = { std::min(P.x, Q.x), std::max(P.x, Q.x) };
					std::pair<float, float> yRange = { std::min(P.y, Q.y), std::max(P.y, Q.y) };
					return std::make_pair(xRange, yRange);
					};

				auto [abX, abY] = getSegmentRange(A, B);
				auto [cdX, cdY] = getSegmentRange(C, D);

				// 投影重叠判定（x和y范围均有交集）
				bool xOverlap = (abX.first <= cdX.second + EPS) && (cdX.first <= abX.second + EPS);
				bool yOverlap = (abY.first <= cdY.second + EPS) && (cdY.first <= abY.second + EPS);

				return xOverlap && yOverlap;
			}

			return crossCondition;
		}

		/**
		 * 核心函数：判断三角形与轴对齐矩形（AABB）是否相交
		 * @param P0 三角形顶点1
		 * @param P1 三角形顶点2
		 * @param P2 三角形顶点3
		 * @param rect_xmin 矩形最小x坐标
		 * @param rect_ymin 矩形最小y坐标
		 * @param rect_xmax 矩形最大x坐标
		 * @param rect_ymax 矩形最大y坐标
		 * @return true：相交（含包含、接触、重叠），false：不相交
		 */
		bool isTriangleAABBIntersect(const Point2D& P0, const Point2D& P1, const Point2D& P2,
			float rect_xmin, float rect_ymin,
			float rect_xmax, float rect_ymax) {
			// 步骤1：快速排斥测试（计算三角形的AABB包围盒）
			float tri_xmin = std::min({ P0.x, P1.x, P2.x });
			float tri_xmax = std::max({ P0.x, P1.x, P2.x });
			float tri_ymin = std::min({ P0.y, P1.y, P2.y });
			float tri_ymax = std::max({ P0.y, P1.y, P2.y });

			// 包围盒不相交的4种情况，直接返回false
			if (tri_xmax < rect_xmin - EPS || tri_xmin > rect_xmax + EPS ||
				tri_ymax < rect_ymin - EPS || tri_ymin > rect_ymax + EPS) {
				return false;
			}

			// 步骤2：场景1：判断三角形任意顶点是否在矩形内
			if (isPointInAABB(P0, rect_xmin, rect_ymin, rect_xmax, rect_ymax) ||
				isPointInAABB(P1, rect_xmin, rect_ymin, rect_xmax, rect_ymax) ||
				isPointInAABB(P2, rect_xmin, rect_ymin, rect_xmax, rect_ymax)) {
				return true;
			}

			// 步骤3：场景2：判断矩形任意顶点是否在三角形内
			Point2D rectPoints[4] = {
				Point2D(rect_xmin, rect_ymin),  // 矩形左下角
				Point2D(rect_xmax, rect_ymin),  // 矩形右下角
				Point2D(rect_xmax, rect_ymax),  // 矩形右上角
				Point2D(rect_xmin, rect_ymax)   // 矩形左上角
			};
			for (int i = 0; i < 4; ++i) {
				if (isPointInTriangle(rectPoints[i], P0, P1, P2)) {
					return true;
				}
			}

			// 步骤4：场景3：判断三角形边与矩形边是否相交
			// 定义三角形的3条边
			std::pair<Point2D, Point2D> triEdges[3] = {
				{P0, P1}, {P1, P2}, {P2, P0}
			};
			// 定义矩形的4条边
			std::pair<Point2D, Point2D> rectEdges[4] = {
				{rectPoints[0], rectPoints[1]},
				{rectPoints[1], rectPoints[2]},
				{rectPoints[2], rectPoints[3]},
				{rectPoints[3], rectPoints[0]}
			};

			// 遍历所有边对，判断是否有相交
			for (const auto& triEdge : triEdges) {
				for (const auto& rectEdge : rectEdges) {
					if (isSegmentIntersect(triEdge.first, triEdge.second,
						rectEdge.first, rectEdge.second)) {
						return true;
					}
				}
			}

			// 所有场景均不满足，返回不相交
			return false;
		}
	}
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
				meshTriangleInfo[mid].baseOffset = baseOffset[mid];
				triangleInfo.insert(triangleInfo.end(), it.second.begin(), it.second.end());
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
	bool BvhService::RayHit(const::Rendering::Geometry::Ray& ray, HitRes& outRes)
	{
		ZoneScoped;
		float triDist = 1e9;
		bool hit = false;
		int mid = -1;
		int tid = -1;
		int instanceId = -1;
		Maths::FVector3 hitNormal;
		Maths::FVector3 bary;
		std::vector<::Rendering::Geometry::Bvh::Node*>stack;
		if (m_sceneBvh != nullptr)
			stack.push_back(m_sceneBvh->m_root);
		while (!stack.empty()) {
			auto cur = stack.back();stack.pop_back();
			if (!cur)continue;
			float tempDist = 1e9;
			if (ray.HitDistance(cur->bounds, tempDist))
			{
				if (cur->type == ::Rendering::Geometry::Bvh::kInternal) {
					stack.push_back(cur->lc);
					stack.push_back(cur->rc);
				}
				else if (cur->type == ::Rendering::Geometry::Bvh::kLeaf) {
					for (int i = cur->startidx;i < cur->startidx + cur->numprims;i++) {
						int index = m_sceneBvh->m_packed_indices[i];
						int meshId = meshInstances[index].meshID;
						auto matrix = meshInstances[index].transform;
						auto invMatrix = Maths::FMatrix4::Inverse(matrix);
						auto localRay = ray.Transformed(invMatrix);
						auto& mesh = meshes[meshId];
						auto meshBvh = mesh->GetBvh();
						std::vector<::Rendering::Geometry::Bvh::Node*>meshBvhStack;
						meshBvhStack.push_back(meshBvh->m_root);
						while (!meshBvhStack.empty()) {
							auto meshBvhCur = meshBvhStack.back(); meshBvhStack.pop_back();
							if (!meshBvhCur)continue;
							float meshTempDist = 1e6;
							if (localRay.HitDistance(meshBvhCur->bounds, meshTempDist))
							{
								if (meshBvhCur->type == ::Rendering::Geometry::Bvh::kInternal) {
									meshBvhStack.push_back(meshBvhCur->lc);
									meshBvhStack.push_back(meshBvhCur->rc);
								}
								else if (meshBvhCur->type == ::Rendering::Geometry::Bvh::kLeaf) {
									for (int j = meshBvhCur->startidx;j < meshBvhCur->startidx + meshBvhCur->numprims;j++) {
										int triIndex = meshBvh->m_packed_indices[j];
										::Rendering::Geometry::VertexBVH v0 = mesh->GetVertexBVH(triIndex * 3);
										::Rendering::Geometry::VertexBVH v1 = mesh->GetVertexBVH(triIndex * 3 + 1);
										::Rendering::Geometry::VertexBVH v2 = mesh->GetVertexBVH(triIndex * 3 + 2);
										float currentTriDist = 1e6;
										Maths::FVector3 currentHitNormal;
										Maths::FVector3 currentBary;
										if (localRay.HitDistance(v0.position, v1.position, v2.position, currentTriDist, &currentHitNormal, &currentBary)) {
											if (currentTriDist < triDist) {
												triDist = currentTriDist;
												hitNormal = currentHitNormal;
												bary = currentBary;
												outRes.hitPoint = v0.position * bary[0] + v1.position * bary[1] + v2.position * bary[2];
												outRes.hitPoint = Maths::FMatrix4::MulPoint(matrix, outRes.hitPoint);
												outRes.hitNormal = Maths::FMatrix4::MulDir(matrix, hitNormal);
												outRes.triangleId = triIndex;
												outRes.actorId = meshInstances[index].actorID;
												outRes.hitUv = v0.texCoords * bary[0] + v1.texCoords * bary[1] + v2.texCoords * bary[2];
												
												hit = true;
												mid = meshId;
												instanceId = index;
												tid = triIndex;//j;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		//if (hit) {
		//	AddTriangleInfo(instanceId, TriangleInfo{ 255u << 24,static_cast<uint32_t>(tid) });
		//}
		return hit;
	}
	bool BvhService::RayIteratorHit(const::Rendering::Geometry::Ray& ray, HitRes& outRes)
	{
		ZoneScoped;
		float triDist = 1e6;
		bool hit = false;
		int mid = -1;
		int instanceId = -1;
		int tid = -1;
		Maths::FVector3 hitNormal;
		Maths::FVector3 bary;
		float tempDist = 1e6;

		if (ray.HitDistance(m_sceneBvh->Bounds(), tempDist)) {
			for (int i = 0;i < meshInstances.size();i++) {
				int meshId = meshInstances[i].meshID;
				auto matrix = meshInstances[i].transform;
				auto invMatrix = Maths::FMatrix4::Inverse(matrix);
				auto localRay = ray.Transformed(invMatrix);
				auto& mesh = meshes[meshId];
				int numTrs=mesh->GetIndexCount()? mesh->GetIndexCount()/3:mesh->GetVertexCount()/3;
				float meshTempDist = 1e6;
				if (localRay.HitDistance(mesh->GetBvh()->Bounds(), meshTempDist))
				{	
					
					for (int triIndex = 0;triIndex < numTrs;triIndex++) {
			
						Maths::FVector3 v0 = mesh->GetVertexPosition(triIndex * 3);
						Maths::FVector3 v1 = mesh->GetVertexPosition(triIndex * 3 + 1);
						Maths::FVector3 v2 = mesh->GetVertexPosition(triIndex * 3 + 2);
						float currentTriDist = 1e6;
						Maths::FVector3 currentHitNormal;
						Maths::FVector3 currentBary;
						if (localRay.HitDistance(v0, v1, v2, currentTriDist, &currentHitNormal, &currentBary)) {
							if (currentTriDist < triDist) {
								triDist = currentTriDist;
								hitNormal = currentHitNormal;
								bary = currentBary;
								outRes.hitPoint = v0 * bary[0] + v1 * bary[1] + v2 * bary[2];
								outRes.hitPoint = Maths::FMatrix4::MulPoint(matrix, outRes.hitPoint);
								outRes.hitNormal = Maths::FMatrix4::MulDir(matrix, hitNormal);
								outRes.triangleId = triIndex;
								outRes.actorId = meshInstances[i].actorID;
								outRes.hitUv = mesh->GetVertexBVH(triIndex * 3).texCoords;
								mid = meshId;
								tid = triIndex;
								instanceId = i;
								hit = true;
							}
						}		
					}
				}
			}	
		}
		//if (hit) {
		//	AddTriangleInfo(instanceId, TriangleInfo{ 255u << 24,static_cast<uint32_t>(tid) });
		//}
		return hit;
	}

	std::vector<RectPickRes> BvhService::RectPick(const Maths::FMatrix4& viewProj, float su, float sv, float eu, float ev)
	{
		ZoneScoped;
		std::vector<RectPickRes> res;
		std::vector<::Rendering::Geometry::Bvh::Node*>stack;
		if (m_sceneBvh != nullptr)
			stack.push_back(m_sceneBvh->m_root);
		while (!stack.empty()) {
			auto cur = stack.back(); stack.pop_back();
			if (!cur)continue;
			auto tbox=cur->bounds.transform(viewProj);
			bool flag1 = tbox.pmin.x > eu || tbox.pmax.x < su;
			bool flag2 = tbox.pmin.y > ev || tbox.pmax.y < sv;
			if (!flag1 && !flag2) {
				if (cur->type == ::Rendering::Geometry::Bvh::kInternal) {
					stack.push_back(cur->lc);
					stack.push_back(cur->rc);
				}
				else if (cur->type == ::Rendering::Geometry::Bvh::kLeaf) {
					for (int i = cur->startidx; i < cur->startidx + cur->numprims; i++) {
						int index = m_sceneBvh->m_packed_indices[i];
						int meshId = meshInstances[index].meshID;
						auto matrix = meshInstances[index].transform;
						auto& mesh = meshes[meshId];
						auto meshBvh = mesh->GetBvh();
						std::vector<::Rendering::Geometry::Bvh::Node*>meshBvhStack;
						meshBvhStack.push_back(meshBvh->m_root);
						while (!meshBvhStack.empty()) {
							auto meshBvhCur = meshBvhStack.back(); meshBvhStack.pop_back();
							if (!meshBvhCur)continue;
							auto mbox=meshBvhCur->bounds.transform(matrix).transform(viewProj);
							if (!(mbox.pmin.x > eu || mbox.pmax.x < su) && !(mbox.pmin.y > ev || mbox.pmax.y < sv)) {
								if (meshBvhCur->type == ::Rendering::Geometry::Bvh::kInternal) {
									meshBvhStack.push_back(meshBvhCur->lc);
									meshBvhStack.push_back(meshBvhCur->rc);
								}
								else if (meshBvhCur->type == ::Rendering::Geometry::Bvh::kLeaf) {
									for (int j = meshBvhCur->startidx; j < meshBvhCur->startidx + meshBvhCur->numprims; j++) {
										int triIndex = meshBvh->m_packed_indices[j];
										::Rendering::Geometry::VertexBVH v0 = mesh->GetVertexBVH(triIndex * 3);
										::Rendering::Geometry::VertexBVH v1 = mesh->GetVertexBVH(triIndex * 3 + 1);
										::Rendering::Geometry::VertexBVH v2 = mesh->GetVertexBVH(triIndex * 3 + 2);
										Maths::FVector3 ndcV0 = viewProj.MulPoint(matrix.MulPoint(v0.position));
										Maths::FVector3 ndcV1 = viewProj.MulPoint(matrix.MulPoint(v1.position));
										Maths::FVector3 ndcV2 = viewProj.MulPoint(matrix.MulPoint(v2.position));
										//test intersection
										if (isTriangleAABBIntersect({ ndcV0.x,ndcV0.y }, { ndcV1.x,ndcV1.y },
											{ ndcV2.x,ndcV2.y }, su, sv, eu, ev)) {
											res.push_back({ (int)meshInstances[index].actorID,(int)v0.texCoords.x,triIndex });
										}
									}
								}
							}
						}
					}
				}
			}
		}
		return res;
	}
	BvhService::~BvhService() {
		delete m_sceneBvh;
	}
}
