#pragma once
#include<string>
#include <vector>
#include "MathUtil.h"
#include <stb_image/stb_image.h>

namespace PathTrace
{
    class EnvironmentMap
    {
    public:
        EnvironmentMap() : width(0), height(0), img(nullptr), cdf(nullptr) {};
        ~EnvironmentMap() { stbi_image_free(img); delete[] cdf; }

        bool LoadMap(const std::string& filename);
        void BuildCDF();

        int width;
        int height;
        //环境的总亮度
        float totalSum;
        //纹理数据(一个像素3字节)
        float* img;
        //纹理亮度累加和序列
        float* cdf;
    };
}
