

#include <iostream>
#include "Texture.h"
#include<glad/glad.h>
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

    bool Texture::LoadTexture(const std::string& filename)
    {

        name = filename;
        components = 4;
        unsigned char* data = stbi_load(filename.c_str(), &width, &height, NULL, components);
        if (data == nullptr){
            throw std::exception("the path is invalid");
        
            return false;
        }
           
         

        glCreateTextures(GL_TEXTURE_2D,1,&id);
        unsigned n_levels=1 + static_cast<GLuint>(floor(std::log2(std::max(width, height))));
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