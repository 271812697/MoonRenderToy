#pragma once
#include <map>
#include <string>
#include <glm/glm.hpp>
#include "../core/base.h"
#include "../asset/all.h"
#include "../component/all.h"
#include "entity.h"
#include "resource.h"
using namespace asset;
using namespace component;

namespace scene {

    class Scene {
      private:
        entt::registry registry;
       
        friend class Renderer;
        
      protected:

        ResourceManager resource_manager;
        std::map<GLuint, UBO> UBOs;  // indexed by uniform buffer's binding point
        std::map<GLuint, FBO> FBOs;  // indexed by the order of creation

        void AddUBO(GLuint shader_id);
        void AddFBO(GLuint width, GLuint height);

        Entity CreateEntity(const std::string& name, ETag tag = ETag::Untagged);
        void DestroyEntity(Entity entity);

      public: 
        std::map<std::string, Entity> directory;
        std::vector<Entity> directory_Entity;
        int selected_entity= -1;
        int cur_operation = 0;
        std::string title;
        explicit Scene(const std::string& title);
        virtual ~Scene();
        virtual void Resize(int w,int h);
        virtual void Present(){}
        int width, height;
        virtual void UpdateScene(float dt = 0);
        virtual void Init(void);
        virtual void OnSceneRender(float dt =0);
        virtual void OnImGuiRender(float dt=0);

      private:

    };

    using glm::vec2, glm::vec3, glm::vec4;
    using glm::mat2, glm::mat3, glm::mat4, glm::quat;
    using glm::ivec2, glm::ivec3, glm::ivec4;
    using glm::uvec2, glm::uvec3, glm::uvec4;
    using uint = unsigned int;

    namespace world {
        // world space constants (OpenGL adopts a right-handed coordinate system)
        static  vec3 origin   { 0.0f };
        static  vec3 zero     { 0.0f };
        static  vec3 unit     { 1.0f };
        static  mat4 identity { 1.0f };
        static  quat eye      { 1.0f, 0.0f, 0.0f, 0.0f };
        static  vec3 up       { 0.0f, 1.0f, 0.0f };
        static  vec3 down     { 0.0f,-1.0f, 0.0f };
        static  vec3 forward  { 0.0f, 0.0f,-1.0f };
        static  vec3 backward { 0.0f, 0.0f, 1.0f };
        static  vec3 left     {-1.0f, 0.0f, 0.0f };
        static  vec3 right    { 1.0f, 0.0f, 0.0f };
    }

    namespace color {
        // some commonly used color presets
        static  vec3 white  { 1.0f };
        static  vec3 black  { 0.0f };
        static  vec3 red    { 1.0f, 0.0f, 0.0f };
        static  vec3 green  { 0.0f, 1.0f, 0.0f };
        static  vec3 lime   { 0.5f, 1.0f, 0.0f };
        static  vec3 blue   { 0.0f, 0.0f, 1.0f };
        static  vec3 cyan   { 0.0f, 1.0f, 1.0f };
        static  vec3 yellow { 1.0f, 1.0f, 0.0f };
        static  vec3 orange { 1.0f, 0.5f, 0.0f };
        static  vec3 purple { 0.5f, 0.0f, 1.0f };
    }

}
