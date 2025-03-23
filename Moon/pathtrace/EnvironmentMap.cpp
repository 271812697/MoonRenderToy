#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <string>
#include <stb_Image/stb_image.h>

#include "MathUtil.h"
#include "EnvironmentMap.h"

namespace PathTrace
{
	float Luminance(float r, float g, float b)
	{
		return 0.212671f * r + 0.715160f * g + 0.072169f * b;
	}

	// https://pbr-book.org/3ed-2018/Light_Transport_I_Surface_Reflection/Sampling_Light_Sources#InfiniteAreaLights
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
		stbi_image_free(img); delete[] cdf;
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