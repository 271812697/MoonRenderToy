
#pragma once

#include <string>
#include <tuple>
#include <vector>
#include <glm/glm.hpp>
#include "component.h"
#include "model.h"

namespace component {

    class Channel {
      private:
        template<typename TKey, int Key>  // strong typedefs
        struct Frame {
            Frame(TKey value, float time) : value(value), timestamp(time) {}
            TKey value;
            float timestamp;
        };

        using FT = Frame<glm::vec3, 1>;  // position frames
        using FR = Frame<glm::quat, 2>;  // rotation frames
        using FS = Frame<glm::vec3, 3>;  // scale frames

      private:
        std::vector<FT> positions;
        std::vector<FR> rotations;
        std::vector<FS> scales;

        template<typename TFrame>
        std::tuple<int, int> GetFrameIndex(const std::vector<TFrame>& frames, float time) const;

      public:
        std::string name;
        int node_id = -1;

        Channel() : node_id(-1) {}
        Channel(aiNodeAnim* ai_channel, const std::string& name, int id, float duration);
        Channel(Channel&& other) = default;
        Channel& operator=(Channel&& other) = default;

        glm::mat4 Interpolate(float time) const;
    };

    class Model;

    class Animation {
      private:
        unsigned int n_channels;
        std::vector<Channel> channels;  // indexed by bone id
        friend class Animator;

      public:
        std::string name;
        float duration;
        float speed;
        Animation(const aiAnimation* ai_animation, Model* model);
        Animation(const aiScene* ai_scene, Model* model);
    };

    class Animator : public Component {
      public:

        float current_time;
        std::vector<glm::mat4> bone_transforms;
        Model* curmodel;
        size_t cur_animation = 0;
        Animator(Model* model);
        void Update(float deltatime);
        inline void Gonext() {
            cur_animation = (cur_animation + 1) % (curmodel->animations.size());
        }

        void Update(Model& model, float deltatime);
        void Reset(Model* model);
    };

}
