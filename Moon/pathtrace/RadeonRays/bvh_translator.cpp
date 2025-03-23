#include <cassert>
#include <stack>
#include <iostream>
#include "bvh_translator.h"

namespace RadeonRays
{
	//BLAS从Mesh层面组织了三角形
	int BvhTranslator::ProcessBLASNodes(const Bvh::Node* node)
	{
		RadeonRays::bbox bbox = node->bounds;

		nodes[curNode].bboxmin = bbox.pmin;
		nodes[curNode].bboxmax = bbox.pmax;
		nodes[curNode].LRLeaf.z = 0;

		int index = curNode;
		//BLAS的叶子节点z=1 x y为二级索引
		if (node->type == RadeonRays::Bvh::NodeType::kLeaf)
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
	//TLAS从场景层面组织了meshInsatnce,至于为什么时meshinstance 因为有Transform,实现了mes的复用
	int BvhTranslator::ProcessTLASNodes(const Bvh::Node* node)
	{
		RadeonRays::bbox bbox = node->bounds;

		nodes[curNode].bboxmin = bbox.pmin;
		nodes[curNode].bboxmax = bbox.pmax;
		nodes[curNode].LRLeaf.z = 0;

		int index = curNode;

		if (node->type == RadeonRays::Bvh::NodeType::kLeaf)
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

	void BvhTranslator::ProcessBLAS()
	{
		int nodeCnt = 0;

		for (int i = 0; i < meshes.size(); i++)
			nodeCnt += meshes[i]->bvh->m_nodecnt;
		topLevelIndex = nodeCnt;

		// reserve space for top level nodes
		nodeCnt += 2 * meshInstances.size();
		nodes.resize(nodeCnt);

		int bvhRootIndex = 0;
		curTriIndex = 0;

		for (int i = 0; i < meshes.size(); i++)
		{
			PathTrace::Mesh* mesh = meshes[i];
			curNode = bvhRootIndex;

			bvhRootStartIndices.push_back(bvhRootIndex);
			bvhRootIndex += mesh->bvh->m_nodecnt;

			ProcessBLASNodes(mesh->bvh->m_root);
			curTriIndex += mesh->bvh->GetNumIndices();
		}
	}

	void BvhTranslator::ProcessTLAS()
	{
		curNode = topLevelIndex;
		ProcessTLASNodes(topLevelBvh->m_root);
	}

	void BvhTranslator::UpdateTLAS(const Bvh* topLevelBvh, const std::vector<PathTrace::MeshInstance>& sceneInstances)
	{
		this->topLevelBvh = topLevelBvh;
		meshInstances = sceneInstances;
		curNode = topLevelIndex;
		ProcessTLASNodes(topLevelBvh->m_root);
	}

	void BvhTranslator::Process(const Bvh* topLevelBvh, const std::vector<PathTrace::Mesh*>& sceneMeshes, const std::vector<PathTrace::MeshInstance>& sceneInstances)
	{
		this->topLevelBvh = topLevelBvh;
		meshes = sceneMeshes;
		meshInstances = sceneInstances;
		ProcessBLAS();
		ProcessTLAS();
	}
}
