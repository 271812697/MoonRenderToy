#pragma once
#include <vector>
#include <string>
namespace PathTrace
{
	struct Vec3;
	class Texture
	{
	public:
		Texture() : width(0), height(0), components(0) {};
		Texture(std::string texName, unsigned char* data, int w, int h, int c);
		~Texture();
		bool LoadTexture(const std::string& filename);

		Vec3 Sample(float u, float v);
		Vec3 Color(int x, int y);
		unsigned int id = 0;
		int width;
		int height;
		int components;
		std::vector<uint8_t> texData;
		std::string name;
	};
}
