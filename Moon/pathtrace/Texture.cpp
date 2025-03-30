
#include <iostream>
#include "Texture.h"
#include "MathUtil.h"
#include <glad/glad.h>
#include <stb_image/stb_image.h>


namespace PathTrace
{
	Texture::Texture(std::string texName, unsigned char* data, int w, int h, int c) : name(texName)
		, width(w)
		, height(h)
		, components(c)
	{
		texData.resize(width * height * components);
		std::copy(data, data + width * height * components, texData.begin());

		glCreateTextures(GL_TEXTURE_2D, 1, &id);
		unsigned n_levels = 1 + static_cast<GLuint>(floor(std::log2(std::max(width, height))));
		glTextureStorage2D(id, n_levels, GL_RGBA8, width, height);
		glTextureSubImage2D(id, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
		if (n_levels > 1) {
			glGenerateTextureMipmap(id);
		}

		glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(id, GL_TEXTURE_WRAP_R, GL_REPEAT);

	}
	Texture::~Texture() {

		glDeleteTextures(1, &id);
	}
	Vec4 Texture::Color(int x, int y) {
		int imgIdx = x * width * 4 + y * 4;
		return Vec4(texData[imgIdx] / 255.0f, texData[imgIdx + 1] / 255.0f, texData[imgIdx + 2] / 255.0f, texData[imgIdx + 3] / 255.0f);
	}
	Vec4 Texture::Sample(float u, float v) {
		float w;
		float alpha;
		alpha = modf(u * (width - 1), &w);

		float h;
		float betha;
		betha = modf(v * (height - 1), &h);
		return (1 - betha) * ((1 - alpha) * this->Color(w, h) + alpha * this->Color(w + 1, h)) + betha * ((1 - alpha) * this->Color(w, h + 1) + alpha * this->Color(w + 1, h + 1));
	}

	bool Texture::LoadTexture(const std::string& filename)
	{

		name = filename;
		components = 4;
		unsigned char* data = stbi_load(filename.c_str(), &width, &height, NULL, components);
		if (data == nullptr) {
			throw std::exception("the path is invalid");

			return false;
		}



		glCreateTextures(GL_TEXTURE_2D, 1, &id);
		unsigned n_levels = 1 + static_cast<GLuint>(floor(std::log2(std::max(width, height))));
		glTextureStorage2D(id, n_levels, GL_RGBA8, width, height);
		glTextureSubImage2D(id, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
		if (n_levels > 1) {
			glGenerateTextureMipmap(id);
		}

		glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(id, GL_TEXTURE_WRAP_R, GL_REPEAT);

		texData.resize(width * height * components);
		std::copy(data, data + width * height * components, texData.begin());
		stbi_image_free(data);


		return true;
	}
}