
#include<glad/glad.h>

#include "Rendering/Buffers/Framebuffer.h"

Rendering::Buffers::Framebuffer::Framebuffer(uint16_t p_width, uint16_t p_height)
	:width(p_width),height(p_height)
{

	/* Generate OpenGL objects */
	glCreateFramebuffers(1, &m_bufferID);
	//AddColorTexture(1, multisample);
    //AddDepStRenderBuffer(multisample);
}

Rendering::Buffers::Framebuffer::~Framebuffer()
{
	/* Destroy OpenGL objects */
	glDeleteFramebuffers(1, &m_bufferID);

    /*
    	for (auto id : color_attachments) {
		glDeleteTextures(1, &id);
	}
    */

	if (m_depthStencilBuffer) {
		glDeleteRenderbuffers(1, &m_depthStencilBuffer);
		m_depthStencilBuffer = 0;
	}
	if (m_deptexture) {
        m_deptexture.release();
		
	}
}

void Rendering::Buffers::Framebuffer::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_bufferID);
}
void Rendering::Buffers::Framebuffer::Clear() const
{
    for (int i = 0; i < color_attachments.size(); i++) {
        Clear(i);
    }

    Clear(-1);
    Clear(-2);
}
void Rendering::Buffers::Framebuffer::Unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Rendering::Buffers::Framebuffer::Resize(uint16_t p_width, uint16_t p_height)
{
	this->width = p_width;
	this->height = p_height;
	int cnt = color_attachments.size();
	color_attachments.clear();
	AddColorTexture(cnt, is_multisample);
	if (m_depthStencilBuffer) {
		glDeleteRenderbuffers(1, &m_depthStencilBuffer);
		m_depthStencilBuffer = 0;
		AddDepStRenderBuffer(is_multisample);
	}

	if (m_deptexture) {
        m_deptexture.release();
		//AddDepStTexture();
	}
}
void Rendering::Buffers::Framebuffer::AddColorTexture(uint16_t count, bool multisample) {
	is_multisample = multisample;
	size_t n_color_buffs = color_attachments.size();
	color_attachments.reserve(n_color_buffs + count);
	for (GLuint i = 0; i < count; i++) {
		GLenum target = multisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
        auto& texture = color_attachments.emplace_back(target, width, height, 1, GL_RGBA16F, 1);
        GLuint tid = texture.id;

        static const float border[] = { 0.0f, 0.0f, 0.0f, 1.0f };

        // we cannot set any of the sampler states for multisampled textures
        if (!multisample) {
            glTextureParameteri(tid, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(tid, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(tid, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTextureParameteri(tid, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTextureParameterfv(tid, GL_TEXTURE_BORDER_COLOR, border);
        }

        glNamedFramebufferTexture(m_bufferID, GL_COLOR_ATTACHMENT0 + n_color_buffs + i, tid, 0);

	}
    SetDrawBuffers();
}
void Rendering::Buffers::Framebuffer::AddDepthCubemap() {


    m_deptexture= std::make_unique<Rendering::Resources::Texture2D>(GL_TEXTURE_CUBE_MAP, width, height, 6, GL_DEPTH_COMPONENT24, 1);
    GLuint tid = m_deptexture->id;

    glTextureParameteri(tid, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT);
    glTextureParameteri(tid, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(tid, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(tid, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(tid, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(tid, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glNamedFramebufferTexture(m_bufferID, GL_DEPTH_ATTACHMENT, tid, 0);
    
    const GLenum null[] = { GL_NONE };
    glNamedFramebufferReadBuffer(m_bufferID, GL_NONE);
    glNamedFramebufferDrawBuffers(m_bufferID, 1, null);
    
    


}
const Rendering::Resources::Texture2D&  Rendering::Buffers::Framebuffer::GetDepthTexture() const{
    return *m_deptexture;
}
void Rendering::Buffers::Framebuffer::AddDepStRenderBuffer(bool multisample) {
	if (m_depthStencilBuffer == 0) {
		is_multisample = multisample;
		glCreateRenderbuffers(1, &m_depthStencilBuffer);
		if (multisample)
		{
			glNamedRenderbufferStorageMultisample(m_depthStencilBuffer,4, GL_DEPTH24_STENCIL8, width, height);
		}
		else {
			glNamedRenderbufferStorage(m_depthStencilBuffer, GL_DEPTH24_STENCIL8, width, height);
		}
		glBindRenderbuffer(GL_RENDERBUFFER, m_depthStencilBuffer);
		glNamedFramebufferRenderbuffer(m_bufferID, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthStencilBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}
}
void Rendering::Buffers::Framebuffer::Clear(GLint index)const {
    const GLfloat clear_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    const GLfloat clear_depth = 1.0f;
    const GLint clear_stencil = 0;

    // clear one of the color attachments
    if (index >= 0) {
        glClearNamedFramebufferfv(m_bufferID, GL_COLOR, index, clear_color);
    }
    // clear the depth buffer
    else if (index == -1) {
        glClearNamedFramebufferfv(m_bufferID, GL_DEPTH, 0, &clear_depth);
    }
    // clear the stencil buffer
    else if (index == -2) {
        glClearNamedFramebufferiv(m_bufferID, GL_STENCIL, 0, &clear_stencil);
    }



}
void Rendering::Buffers::Framebuffer::SetDrawBuffers() const
{
    if (size_t n = color_attachments.size(); n > 0) {
        GLenum* attachments = new GLenum[n];
        for (GLenum i = 0; i < n; i++) {
            *(attachments + i) = GL_COLOR_ATTACHMENT0 + i;
        }
        glNamedFramebufferDrawBuffers(m_bufferID, n, attachments);
        delete[] attachments;
    }
}
void Rendering::Buffers::Framebuffer::CopyColor(const Framebuffer& fr, GLuint fr_idx, const Framebuffer& to, GLuint to_idx)
{

    // if the source and target rectangle areas differ in size, interpolation will be applied
    GLuint fw = fr.width, fh = fr.height;
    GLuint tw = to.width, th = to.height;

    glNamedFramebufferReadBuffer(fr.GetID(), GL_COLOR_ATTACHMENT0 + fr_idx);
    glNamedFramebufferDrawBuffer(to.GetID(), GL_COLOR_ATTACHMENT0 + to_idx);
    glBlitNamedFramebuffer(fr.GetID(), to.GetID(), 0, 0, fw, fh, 0, 0, tw, th, GL_COLOR_BUFFER_BIT, GL_NEAREST);

}
uint32_t Rendering::Buffers::Framebuffer::GetID() const
{
    return m_bufferID;
}


uint32_t Rendering::Buffers::Framebuffer::GetTextureID(uint16_t index)
{
	if(index<color_attachments.size())
	return color_attachments.at(index).id;
	return 0;
}
const Rendering::Resources::Texture2D& Rendering::Buffers::Framebuffer::GetColorTexture(GLenum index) const
{
  
    return color_attachments[index];
}

uint32_t Rendering::Buffers::Framebuffer::GetRenderBufferID()
{
	return m_depthStencilBuffer;
}
