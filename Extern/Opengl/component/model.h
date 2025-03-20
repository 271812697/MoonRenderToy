

#pragma once

#include <bitset>
#include <string>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../core/base.h"
#include "component.h"
#include"../asset/all.h"
using namespace asset;
namespace component {

    class Animation;
    class Mesh;
    class Material;

    class Node {
        
      public:
        Node(int nid, int pid, const std::string& name);
        bool IsBone() const;
        bool Animated() const;

        int nid = -1;    // node id, must >= 0
        int pid = -1;    // node id of the parent, must < nid, for the root node, this is -1
        int bid = -1;    // bone id, if node is not a bone node, this is -1
        bool alive = 0;  // is bone node && influenced by a channel

        std::string name;
        glm::mat4 n2p;   // node space -> parent space (local transform relative to the parent)
        glm::mat4 m2n;   // model space (bind pose) -> node space (bone space), for bone nodes only
        glm::mat4 n2m;   // bone space -> model space, updated at runtime, N/A if not alive
    };

    enum class Quality : uint32_t {  // import quality preset
        Auto   = 0x0,
        Low    = aiProcessPreset_TargetRealtime_Fast,
        Medium = aiProcessPreset_TargetRealtime_Quality,
        High   = aiProcessPreset_TargetRealtime_MaxQuality
    };

    class Model : public Component {
      private:
        const aiScene* ai_root = nullptr;
        std::bitset<6> vtx_format;
        std::unordered_map<std::string, unsigned int> materials_cache;  // matkey : matid

      public:
        unsigned int n_nodes = 0, n_bones = 0;
        unsigned int n_meshes = 0, n_verts = 0, n_tris = 0;
        bool animated = false;
        std::filesystem::path directory;
        std::vector<Node> nodes;
        std::vector<Mesh> meshes;
        std::unordered_map<unsigned int, asset_ref<Texture>>Texture_Diffuse;
        //std::unordered_map<unsigned int, asset_ref<Texture>>Texture_;
        std::unordered_map<unsigned int, Material> materials;  // matid : material
        std::shared_ptr<Animation> animation;
        std::vector<std::shared_ptr<Animation>>animations;

      private:
        void ProcessTree(aiNode* ai_node, int parent);
        void ProcessNode(aiNode* ai_node);
        void ProcessMesh(aiMesh* ai_mesh);
        void ProcessMaterial(aiMaterial* ai_material, const Mesh& mesh);

      public:
        Model(const std::string& filepath, Quality quality, bool animate = false);
        Material& SetMaterial(const std::string& matkey, asset_ref<Material>&& material);
        void AttachMotion(const std::string& filepath);
    };

}
