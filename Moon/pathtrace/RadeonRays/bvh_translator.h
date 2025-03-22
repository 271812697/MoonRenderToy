//	Modified version of code from https://github.com/GPUOpen-LibrariesAndSDKs/RadeonRays_SDK 

#pragma once
#ifndef BVH_TRANSLATOR_H
#define BVH_TRANSLATOR_H

#include <map>
#include "bvh.h"
#include "../Mesh.h"

namespace RadeonRays
{
    /// This class translates pointer based BVH representation into
    /// index based one suitable for feeding to GPU or any other accelerator
    //
    class BvhTranslator
    {
    public:
        // Constructor
        BvhTranslator() = default;

        struct Node
        {
            Vec3 bboxmin;
            Vec3 bboxmax;
            Vec3 LRLeaf;
        };

        void ProcessBLAS();
        void ProcessTLAS();
        void UpdateTLAS(const Bvh* topLevelBvh, const std::vector<PathTrace::MeshInstance>& instances);
        void Process(const Bvh* topLevelBvh, const std::vector<PathTrace::Mesh*>& meshes, const std::vector<PathTrace::MeshInstance>& instances);
        int topLevelIndex = 0;
        std::vector<Node> nodes;
        int nodeTexWidth;

    private:
        int curNode = 0;
        int curTriIndex = 0;
        std::vector<int> bvhRootStartIndices;
        int ProcessBLASNodes(const Bvh::Node* root);
        int ProcessTLASNodes(const Bvh::Node* root);
        std::vector<PathTrace::MeshInstance> meshInstances;
        std::vector<PathTrace::Mesh*> meshes;
        const Bvh* topLevelBvh;
    };
}

#endif // BVH_TRANSLATOR_H
