#pragma once

#include <vector>
#include<glad/glad.h>
#include "RadeonRays/split_bvh.h"

namespace PathTrace
{
    class Mesh
    {
    public:
        Mesh()
        {
            bvh = new RadeonRays::SplitBvh(2.0f, 64, 0, 0.001f, 0);
            
        }
        ~Mesh() { delete bvh; glDeleteBuffers(1,&vao); glDeleteBuffers(1, &vbop); glDeleteBuffers(1, &vbon);
        
        }
        void GenVAO();
        void BuildBVH();
        bool LoadFromFile(const std::string& filename);
        void Draw();

        //存储法线顶点坐标 uv分开存
        std::vector<Vec4> verticesUVX; // Vertex + texture Coord (u/s)
        std::vector<Vec4> normalsUVY;  // Normal + texture Coord (v/t)
        GLuint vao;
        GLuint vbop;
        GLuint vbon;

        RadeonRays::Bvh* bvh;
        std::string name;
    };

    class MeshInstance
    {

    public:
        MeshInstance(std::string name, int meshId, Mat4 xform, int matId)
            : name(name)
            , meshID(meshId)
            , transform(xform)
            , materialID(matId)
        {
            parentID = -1;
        }
        ~MeshInstance() {}

        Mat4 transform;
        Mat4 localform;
        std::string name;

        int materialID;
        int meshID;
        int parentID;
    };
}

