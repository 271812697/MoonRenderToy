#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <string>
#include <stb_Image/stb_image.h>

#include "MathUtil.h"
#include "EnvironmentMap.h"

namespace PathTrace
{

	void EnvironmentMap::BuildCDF()
	{
		// Gather weights for CDF
		float* weights = new float[width * height];
		for (int v = 0; v < height; v++)
		{
			for (int u = 0; u < width; u++)
			{
				int imgIdx = v * width * 3 + u * 3;
				weights[u + v * width] = Luminance(img[imgIdx + 0], img[imgIdx + 1], img[imgIdx + 2]);
			}
		}

		// Build CDF
		cdf = new float[width * height];
		cdf[0] = weights[0];
		for (int i = 1; i < width * height; i++)
			cdf[i] = cdf[i - 1] + weights[i];

		totalSum = cdf[width * height - 1];
		delete[] weights;
	}

	EnvironmentMap::~EnvironmentMap()
	{
		stbi_image_free(img);
		delete[] cdf;
	}
	Vec3 EnvironmentMap::Sample(float u, float v) {
		float w;
		float alpha;
		alpha = modf(u * (width - 1), &w);

		float h;
		float betha;
		betha = modf(v * (height - 1), &h);
		return (1 - betha) * ((1 - alpha) * this->Color(w, h) + alpha * this->Color(w + 1, h)) + betha * ((1 - alpha) * this->Color(w, h + 1) + alpha * this->Color(w + 1, h + 1));
	}
	Vec3 EnvironmentMap::Color(int x, int y) {
		int imgIdx = x * width * 3 + y * 3;
		return Vec3(img[imgIdx + 0], img[imgIdx + 1], img[imgIdx + 2]);
	}
	Vec3 EnvironmentMap::Sample(float u, Vec2* out)
	{
		float value = totalSum * u;
		int left = 0;
		int right = width * height - 1;
		int mid = 0;
		while (left < right) {
			mid = (left + right) / 2;
			if (cdf[mid] < value) {
				left = mid + 1;
			}
			else {
				right = mid;
			}
		}
		mid = (left + right) / 2;
		int row = mid / width;
		int col = mid % width;
		*out = { 1.0f * row / height ,1.0f * col / width };
		//clamp?
		return Color(row, col);
	}
	bool EnvironmentMap::LoadMap(const std::string& filename)
	{
		img = stbi_loadf(filename.c_str(), &width, &height, NULL, 3);
		if (img == nullptr)
			return false;
		BuildCDF();
		return true;
	}
}