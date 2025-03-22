
#pragma once

#include <vector>
#include <algorithm>

namespace PathTrace
{
    class Texture
    {
    public:
        Texture() : width(0), height(0), components(0) {};
        Texture(std::string texName, unsigned char* data, int w, int h, int c);
        ~Texture();
        bool LoadTexture(const std::string& filename);

        unsigned int id=0;
        int width;
        int height;
        int components;
        std::vector<unsigned char> texData;
        std::string name;
    };
}
