#pragma once

#include <vector>
#include "texture.h"
#include"../pch.h"
namespace asset {

    class RBO : public IAsset {
    public:
        bool is_multisample = false;
        RBO(GLuint width, GLuint height, bool multisample = false);
        ~RBO();
        void Resize(GLuint width, GLuint height);
        void Bind() const override;
        void Unbind() const override;
    };

    class FBO : public IAsset {
    private:
        GLenum status;


        std::vector<Texture> color_attachments;   // vector of color attachments
        std::unique_ptr<RBO>       depst_renderbuffer;  // depth and stencil as a single renderbuffer
        std::unique_ptr <Texture>   depst_texture;       // depth and stencil as a single texture
        std::unique_ptr<TexView>   stencil_view;        // stencil as a temporary texture view

    public:
        GLuint width, height;
        FBO() = default;
        FBO(GLuint width, GLuint height);
        ~FBO();

        FBO(const FBO&);
        FBO& operator=(const FBO&);
        FBO(FBO&& other) noexcept = default;
        FBO& operator=(FBO&& other) noexcept = default;

    public:
        void Resize(GLuint width, GLuint height);
        void AddColorTexture(GLuint count, bool multisample = false);
        void SetColorTexture(GLenum index, GLuint texture_2d);
        void SetColorTexture(GLenum index, GLuint texture_cubemap, GLuint face);
        void AddDepStTexture(bool multisample = false);
        void AddDepStRenderBuffer(bool multisample = false);
        void AddDepthCubemap();

        const Texture& GetColorTexture(GLenum index) const;
        const Texture& GetDepthTexture() const;
        const TexView& GetStencilTexView() const;

        void Bind() const override;
        void Unbind() const override;

        void SetDrawBuffer(GLuint index) const;
        void SetDrawBuffers(std::vector<GLuint> indices) const;
        void SetDrawBuffers() const;

        void Draw(GLint index) const;
        void Clear(GLint index) const;
        void Clear() const;

    public:
        static void CopyColor(const FBO& fr, GLuint fr_idx, const FBO& to, GLuint to_idx);
        static void CopyDepth(const FBO& fr, const FBO& to);
        static void CopyStencil(const FBO& fr, const FBO& to);
    };

}