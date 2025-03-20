

#pragma once

#include <string>
#include <glad/glad.h>
#include "asset.h"

namespace asset {


    class Texture;  // forward declaration

    class TexView : public IAsset {
    public:
        const Texture& host;
        TexView(const Texture& texture);
        TexView(const Texture&& texture) = delete;  // prevent rvalue references to temporary objects
        ~TexView();

        TexView(const TexView&) = delete;
        TexView& operator=(const TexView&) = delete;
        TexView(TexView&& other) = default;
        TexView& operator=(TexView&& other) = default;

        void SetView(GLenum target, GLuint fr_level, GLuint levels, GLuint fr_layer, GLuint layers) const;
        void Bind(GLuint index) const;
        void Unbind(GLuint index) const;
    };

    class Texture : public IAsset {
    public:
        friend class TexView;
        GLenum target;
        GLenum format, i_format;  // internal format
        void SetSampleState() const;

    public:
        GLuint width, height, depth;
        GLuint n_levels;

        Texture(const std::string& img_path, GLuint levels = 0);
        Texture(const std::string& img_path, GLuint resolution, GLuint levels);
        Texture(const std::string& directory, const std::string& extension, GLuint resolution, GLuint levels);
        Texture(GLenum target, GLuint width, GLuint height, GLuint depth, GLenum i_format, GLuint levels);
        ~Texture();

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        Texture(Texture&& other) noexcept = default;
        Texture& operator=(Texture&& other) noexcept = default;
        void Resize(GLuint width, GLuint height);
        void Bind(GLuint index) const override;
        void Unbind(GLuint index) const override;
        void BindILS(GLuint level, GLuint index, GLenum access) const;
        void UnbindILS(GLuint index) const;

        void GenerateMipmap() const;
        void Clear(GLuint level) const;
        void Invalidate(GLuint level) const;

        static void Copy(const Texture& fr, GLuint fr_level, const Texture& to, GLuint to_level);
    };

}