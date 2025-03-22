#pragma once

#include"Opengl/asset/shader.h"
namespace PathTrace
{

    class Quad
    {
    public:
        Quad();
        void Draw(asset::Shader* shader);

    private:
        GLuint vao;
        GLuint vbo;
    };
}