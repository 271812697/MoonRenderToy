

#pragma once

#include <string>
#include <queue>
#include <ECS/entt.hpp>
#include "../asset/shader.h"

namespace scene {

    class Scene;  // forward declaration

    class Renderer {
      private:
        static Scene* last_scene;
        static Scene* curr_scene;
        static std::queue<entt::entity> render_queue;

      public:
        static const Scene* GetScene();
        static void SetScene(Scene* s);
        // configuration functions
        static void MSAA(bool enable);
        static void DepthPrepass(bool enable);
        static void DepthTest(bool enable);
        static void StencilTest(bool enable);
        static void AlphaBlend(bool enable);
        static void FaceCulling(bool enable);
        static void SeamlessCubemap(bool enable);
        static void PrimitiveRestart(bool enable);
        static void SetFrontFace(bool ccw);
        static void SetViewport(GLuint width, GLuint height);
        static void SetShadowPass(unsigned int index);

        // core event functions
        static void Attach(const std::string& title);
        static void Detach();

        static void Reset();
        static void Clear();
        static void Flush();
        static void Render(const asset_ref<asset::Shader> custom_shader = nullptr);

        static void DrawScene();
        static void DrawImGui();

        // submit a variable number of entity ids to the render queue
        template<typename... Args>
        static void Submit(Args&&... args) {
            (render_queue.push(args), ...);
        }
    };

}
