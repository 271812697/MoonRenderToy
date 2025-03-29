#pragma once
#include<string>
namespace PathTrace
{
	struct Vec3;
	struct Vec2;
	class EnvironmentMap
	{
	public:
		EnvironmentMap() : width(0), height(0), img(nullptr), cdf(nullptr) {};
		~EnvironmentMap();

		bool LoadMap(const std::string& filename);
		void BuildCDF();

		Vec3 Sample(float u, float v);
		Vec3 Color(int x, int y);
		//importance sample
		Vec3 Sample(float u, Vec2* out);
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
