#pragma once
#include<string>



namespace PathTrace
{
	class EnvironmentMap
	{
	public:
		EnvironmentMap() : width(0), height(0), img(nullptr), cdf(nullptr) {};
		~EnvironmentMap();

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
