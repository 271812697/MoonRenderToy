#include"Rendering/Resources/Texture2D.h"
#include<filesystem>
#include<vector>
#include<cmath>
#include<glad/glad.h>
#include <stb_image/stb_image.h>
namespace Rendering::Resources
{
    static std::string code = R"(
#version 460 core
#define PI       3.141592653589793
#define PI2      6.283185307179586
layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
layout(binding = 0) uniform sampler2D equirectangle;
layout(binding = 0, rgba16f) restrict writeonly uniform imageCube cubemap;


vec3 UV2Cartesian(vec2 st, uint face) {
    vec3 v = vec3(0.0);  // texture lookup vector in world space
    vec2 uv = 2.0 * vec2(st.x, 1.0 - st.y) - 1.0;  // convert [0, 1] to [-1, 1] and invert y

    // https://en.wikipedia.org/wiki/Cube_mapping#Memory_addressing
    switch (face) {
        case 0: v = vec3( +1.0,  uv.y, -uv.x); break;  // posx
        case 1: v = vec3( -1.0,  uv.y,  uv.x); break;  // negx
        case 2: v = vec3( uv.x,  +1.0, -uv.y); break;  // posy
        case 3: v = vec3( uv.x,  -1.0,  uv.y); break;  // negy
        case 4: v = vec3( uv.x,  uv.y,  +1.0); break;  // posz
        case 5: v = vec3(-uv.x,  uv.y,  -1.0); break;  // negz
    }

    return normalize(v);
}

// convert an ILS image coordinate w to its equivalent 3D texture lookup
// vector v such that `texture(samplerCube, v) == imageLoad(imageCube, w)`
vec3 ILS2Cartesian(ivec3 w, vec2 resolution) {
    // w often comes from a compute shader in the form of `gl_GlobalInvocationID`
    vec2 st = w.xy / resolution;  // tex coordinates in [0, 1] range
    return UV2Cartesian(st, w.z);
}


// remap a spherical vector v into equirectangle texture coordinates
vec2 Spherical2Equirect(vec2 v) {
    return vec2(v.x + 0.5, v.y);  // ~ [0, 1]
}


// convert a vector v in Cartesian coordinates to spherical coordinates
vec2 Cartesian2Spherical(vec3 v) {
    float phi = atan(v.z, v.x);          // ~ [-PI, PI] (assume v is normalized)
    float theta = acos(v.y);             // ~ [0, PI]
    return vec2(phi / PI2, theta / PI);  // ~ [-0.5, 0.5], [0, 1]
}

void main() {
    vec2 resolution = vec2(imageSize(cubemap));
    ivec3 ils_coordinate = ivec3(gl_GlobalInvocationID);

    vec3 v = ILS2Cartesian(ils_coordinate, resolution);

    vec2 sample_vec = Cartesian2Spherical(v);
    sample_vec = Spherical2Equirect(sample_vec);
    vec4 color = texture(equirectangle, sample_vec);

    imageStore(cubemap, ils_coordinate, color);
}
)";
    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    static constexpr bool IsPowerOfTwo(T value) {  // implicitly inline
        return value != 0 && (value & (value - 1)) == 0;
    }

    class Image {
    private:
        GLint width, height, n_channels;
        bool is_hdr;

        struct deleter {
            void operator()(uint8_t* buffer);
        };

        std::unique_ptr<uint8_t, deleter> pixels;  // with `stb` custom deleter

    public:
        Image(const std::string& filepath, GLuint channels = 0, bool flip = false);

        Image(const Image&) = delete;  // `std::unique_ptr` is move-only
        Image& operator=(const Image&) = delete;
        Image(Image&& other) noexcept = default;
        Image& operator=(Image&& other) noexcept = default;

        bool IsHDR() const;
        GLuint Width() const;
        GLuint Height() const;
        GLenum Format() const;
        GLenum IFormat() const;

        template<typename T>
        const T* GetPixels() const;
    };
    void Image::deleter::operator()(uint8_t* buffer) {
        if (buffer != nullptr) {
            stbi_image_free(buffer);
        }
    }

    Image::Image(const std::string& filepath, GLuint channels, bool flip) : width(0), height(0), n_channels(0) {
        stbi_set_flip_vertically_on_load(flip);
        this->is_hdr = stbi_is_hdr(filepath.c_str());
        if (is_hdr) {
            float* buffer = stbi_loadf(filepath.c_str(), &width, &height, &n_channels, 4);
            pixels.reset(reinterpret_cast<uint8_t*>(buffer));
        }
        else {
            uint8_t* buffer = stbi_load(filepath.c_str(), &width, &height, &n_channels, channels);
            if (buffer == nullptr) {
                return;
            }
            pixels.reset(buffer);
        }
     
        if (pixels == nullptr) {
            throw std::runtime_error("Unable to claim image data from: " + filepath);
        }
    }

    bool Image::IsHDR() const {
        return is_hdr;
    }

    GLuint Image::Width() const {
        return static_cast<GLuint>(width);
    }

    GLuint Image::Height() const {
        return static_cast<GLuint>(height);
    }

    GLenum Image::Format() const {
        if (is_hdr) {
            return GL_RGBA;
        }

        switch (n_channels) {
        case 1:  return GL_RED;  // greyscale
        case 2:  return GL_RG;   // greyscale + alpha
        case 3:  return GL_RGB;
        case 4:  return GL_RGBA;
        default: return 0;
        }
    }

    GLenum Image::IFormat() const {
        if (is_hdr) {
            return GL_RGBA16F;
        }

        switch (n_channels) {
        case 1:  return GL_R8;   // greyscale
        case 2:  return GL_RG8;  // greyscale + alpha
        case 3:  return GL_RGB8;
        case 4:  return GL_RGBA8;
        default: return 0;
        }
    }

    template<typename T>
    const T* Image::GetPixels() const {
        return reinterpret_cast<const T*>(pixels.get());
    }

    // explicit template function instantiation
    template const uint8_t* Image::GetPixels<uint8_t>() const;
    template const float* Image::GetPixels<float>() const;
    Texture2D::Texture2D(const std::string& img_path, unsigned int levels)
        :  target(GL_TEXTURE_2D), depth(1), n_levels(levels)
    {
        auto image = Image(img_path);

        this->width = image.Width();
        this->height = image.Height();
        this->format = image.Format();
        this->i_format = image.IFormat();

        // if levels is 0, automatic compute the number of mipmap levels needed
        if (levels == 0) {
            n_levels = 1 + static_cast<unsigned int>(floor(std::log2(std::max(width, height))));
        }

        glCreateTextures(GL_TEXTURE_2D, 1, &id);
        glTextureStorage2D(id, n_levels, i_format, width, height);

        if (image.IsHDR()) {
            glTextureSubImage2D(id, 0, 0, 0, width, height, format, GL_FLOAT, image.GetPixels<float>());
        }
        else {
            glTextureSubImage2D(id, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, image.GetPixels<uint8_t>());
        }

        if (n_levels > 1) {
            glGenerateTextureMipmap(id);
        }

        SetSampleState();
    }
    //get a cubemap from hdr format
    Texture2D::Texture2D(const std::string& img_path, unsigned int resolution, unsigned int levels)
        :  target(GL_TEXTURE_CUBE_MAP), width(resolution), height(resolution), depth(6), n_levels(levels)
    {
        // resolution must be a power of 2 in order to achieve high-fidelity visual effects
        if (!IsPowerOfTwo(resolution)) {

            return;
        }

        // a cubemap texture should be preferably created from a high dynamic range image
        if (auto path = std::filesystem::path(img_path); path.extension() != ".hdr") {

        }

        // image load store does not allow 3-channel formats, we have to use GL_RGBA
        this->format = GL_RGBA;
        this->i_format = GL_RGBA16F;

        if (levels == 0) {
            n_levels = 1 + static_cast<unsigned int>(floor(std::log2(std::max(width, height))));
        }

        // load the equirectangular image into a temporary 2D texture (base level, no mipmaps)
        unsigned int equirectangle = 0;
        glCreateTextures(GL_TEXTURE_2D, 1, &equirectangle);

        if (equirectangle > 0) {
            auto image = Image(img_path, 3);

            unsigned int im_w = image.Width();
            unsigned int im_h = image.Height();
            unsigned int im_fmt = image.Format();
            GLenum im_ifmt = image.IFormat();

            glTextureParameteri(equirectangle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(equirectangle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTextureParameteri(equirectangle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(equirectangle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

            if (image.IsHDR()) {
                glTextureStorage2D(equirectangle, 1, im_ifmt, im_w, im_h);
                glTextureSubImage2D(equirectangle, 0, 0, 0, im_w, im_h, im_fmt, GL_FLOAT, image.GetPixels<float>());
            }
            else {
                glTextureStorage2D(equirectangle, 1, im_ifmt, im_w, im_h);
                glTextureSubImage2D(equirectangle, 0, 0, 0, im_w, im_h, im_fmt, GL_UNSIGNED_BYTE, image.GetPixels<uint8_t>());
            }
        }

        // create this texture as an empty cubemap to hold the equirectangle
        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &id);
        glTextureStorage2D(id, n_levels, i_format, width, height);

        // project the 2D equirectangle onto the six faces of our cubemap using compute shader
        //CORE_INFO("Creating cubemap from {0}", img_path);

        const char* c_source_code = code.c_str();
        GLuint shader_id = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(shader_id, 1, &c_source_code, nullptr);
        glCompileShader(shader_id);
        GLuint pid = glCreateProgram();
        glAttachShader(pid, shader_id);
        glLinkProgram(pid);
        glDetachShader(pid, shader_id);
        glDeleteShader(shader_id);
      
        if (true) {
            glUseProgram(pid);
            glBindTextureUnit(0, equirectangle);
            glBindImageTexture(0, id, 0, GL_TRUE, 0, GL_WRITE_ONLY, i_format);
            glDispatchCompute(resolution / 32, resolution / 32, 6);  // six faces
            glMemoryBarrier(GL_ALL_BARRIER_BITS);  // sync wait
            glBindTextureUnit(0, 0);
            glBindImageTexture(0, 0, 0, GL_TRUE, 0, GL_WRITE_ONLY, i_format);
            glUseProgram(0);
        }
        glDeleteProgram(pid);

        glDeleteTextures(1, &equirectangle);  // delete the temporary 2D equirectangle texture

        if (n_levels > 1) {
            glGenerateTextureMipmap(id);
        }

        SetSampleState();
    }

    Texture2D::Texture2D(const std::string& directory, const std::string& extension, unsigned int resolution, unsigned int levels)
        :  target(GL_TEXTURE_CUBE_MAP), width(resolution), height(resolution),
        depth(6), format(GL_RGBA), i_format(GL_RGBA16F), n_levels(levels)
    {
        // resolution must be a power of 2 in order to achieve high-fidelity visual effects
        if (!IsPowerOfTwo(resolution)) {
            //CORE_ERROR("Attempting to build a cubemap whose resolution is not a power of 2...");
            return;
        }

        // this ctor expects 6 HDR images for the 6 cubemap faces, named as follows
        static const std::vector<std::string> faces{ "px", "nx", "py", "ny", "pz", "nz" };

        // the stb image library currently does not support ".exr" format ...
       // CORE_ASERT(extension == ".hdr", "Invalid file extension, expected HDR-format faces...");

        std::string test_face = directory + faces[0] + extension;
        if (!std::filesystem::exists(std::filesystem::path(test_face))) {
            //CORE_ERROR("Cannot find cubemap face {0} in the directory...", test_face);
            return;
        }

        if (levels == 0) {
            n_levels = 1 + static_cast<unsigned int>(floor(std::log2(std::max(width, height))));
        }

        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &id);
        glTextureStorage2D(id, n_levels, i_format, width, height);

        for (unsigned int face = 0; face < 6; face++) {
            auto image = Image(directory + faces[face] + extension, 3, true);
            glTextureSubImage3D(id, 0, 0, 0, face, width, height, 1, format, GL_FLOAT, image.GetPixels<float>());
        }

        if (n_levels > 1) {
            glGenerateTextureMipmap(id);
        }
        //(unsigned int texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels)
        SetSampleState();
    }

    Texture2D::Texture2D(GLenum target, unsigned int width, unsigned int height, unsigned int depth, GLenum i_format, unsigned int levels)
        :  target(target), width(width), height(height), depth(depth),
        n_levels(levels), format(0), i_format(i_format)
    {
        if (levels == 0) {
            n_levels = 1 + static_cast<unsigned int>(floor(std::log2(std::max(width, height))));
        }

        // TODO: deduce format from i_format

        glCreateTextures(target, 1, &id);

        switch (target) {
        case GL_TEXTURE_2D:
        case GL_TEXTURE_CUBE_MAP: {  // depth must = 6
            glTextureStorage2D(id, n_levels, i_format, width, height);
            break;
        }
        case GL_TEXTURE_2D_MULTISAMPLE: {
            glTextureStorage2DMultisample(id, 4, i_format, width, height, GL_TRUE);
            break;
        }
        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_CUBE_MAP_ARRAY: {  // depth must = 6 * n_layers
            glTextureStorage3D(id, n_levels, i_format, width, height, depth);
            break;
        }
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY: {
            glTextureStorage3DMultisample(id, 4, i_format, width, height, depth, GL_TRUE);
            break;
        }
        default: {
            throw ("Unsupported texture target...");
        }
        }

        SetSampleState();
    }

    Texture2D::~Texture2D() {
        if (id == 0) return;

        glDeleteTextures(1, &id);  // texture 0 (a fallback texture that is all black) is silently ignored

    }

    void Texture2D::Resize(unsigned int width, unsigned int height)
    {
       // CORE_ASERT(target == GL_TEXTURE_2D || target == GL_TEXTURE_2D_MULTISAMPLE, "Invalid Format to Resize");
        if (target == GL_TEXTURE_2D)
            glTextureStorage2D(id, n_levels, i_format, width, height);
        else
            glTextureStorage2DMultisample(id, 4, i_format, width, height, GL_TRUE);
    }

    void Texture2D::Bind(unsigned int index) const {

            glBindTextureUnit(index, id);

    }

    void Texture2D::Unbind(unsigned int index) const {
 
            glBindTextureUnit(index, 0);
 
    }

    void Texture2D::BindILS(unsigned int level, unsigned int index, GLenum access) const {

        glBindImageTexture(index, id, level, GL_TRUE, 0, access, i_format);
    }

    void Texture2D::UnbindILS(unsigned int index) const {
        glBindImageTexture(index, 0, 0, GL_TRUE, 0, GL_READ_ONLY, i_format);
    }

    void Texture2D::GenerateMipmap() const {
        
        glGenerateTextureMipmap(id);
    }

    void Texture2D::Clear(unsigned int level) const {
        switch (i_format) {
        case GL_RG16F: case GL_RGB16F: case GL_RGBA16F:
        case GL_RG32F: case GL_RGB32F: case GL_RGBA32F: {
            glClearTexSubImage(id, level, 0, 0, 0, width, height, depth, format, GL_FLOAT, NULL);
            break;
        }
        default: {
            glClearTexSubImage(id, level, 0, 0, 0, width, height, depth, format, GL_UNSIGNED_BYTE, NULL);
            return;
        }
        }
    }

    void Texture2D::Invalidate(unsigned int level) const {
        glInvalidateTexSubImage(id, level, 0, 0, 0, width, height, depth);
    }

    void Texture2D::SetSampleState() const {
        // for magnification, bilinear filtering is more than enough, for minification,
        // trilinear filtering is only necessary when we need to sample across mipmaps
        GLint mag_filter = GL_LINEAR;
        GLint min_filter = n_levels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;

        // anisotropic filtering requires OpenGL 4.6, where maximum anisotropy is implementation-defined
        static GLfloat anisotropy = -1.0f;
        if (anisotropy < 0) {
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &anisotropy);
            anisotropy = anisotropy < 1.0f ? 1.0f :anisotropy>8.0f?8.0f: anisotropy;// std::clamp(anisotropy, 1.0f, 8.0f);  // limit anisotropy to 8
        }

        switch (target) {
        case GL_TEXTURE_2D:
        case GL_TEXTURE_2D_ARRAY: {
            if (i_format == GL_RG16F) {  // 2D BRDF LUT, inverse LUT, fake BRDF maps, etc
                glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }
            else if (i_format == GL_RGB16F) {  // 3D BRDF LUT, cloth DFG LUT, etc
                glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTextureParameteri(id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            }
            else if (i_format == GL_RGBA16F) {  // 3D BRDF DFG LUT used as ILS (uniform image2D)
                glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTextureParameteri(id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            }
            else if (i_format == GL_DEPTH_COMPONENT) {  // depth texture and shadow maps
                glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            }
            else {
                // the rest of 2D textures are mostly normal and seamless so just repeat, but be aware
                // that some of those with a GL_RGBA format are intended for alpha blending so must be
                // clamped to edge instead. However, checking `format == GL_RGBA` and alpha < 1 is not
                // enough to conclude, it all depends, in that case we need to set wrap mode manually
                glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, min_filter);
                glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, mag_filter);
                glTextureParameterf(id, GL_TEXTURE_MAX_ANISOTROPY, anisotropy);
            }
            break;
        }
        case GL_TEXTURE_2D_MULTISAMPLE:
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY: {
            // multisampled textures are not filtered at all, there's nothing we need to do here because
            // we'll never sample them, the hardware takes care of all the multisample operations for us
            // in fact, trying to set any of the sampler states will cause a `GL_INVALID_ENUM` error.
            return;
        }
        case GL_TEXTURE_CUBE_MAP:  // skybox and IBL maps
        case GL_TEXTURE_CUBE_MAP_ARRAY: {
            glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, min_filter);
            glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, mag_filter);
            glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTextureParameteri(id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
            const float border[] = { 0.0f, 0.0f, 0.0f, 1.0f };
            glTextureParameterfv(id, GL_TEXTURE_BORDER_COLOR, border);
            break;
        }
        default: {
            throw ("Unsupported texture target...");
        }
        }
    }

    void Texture2D::Copy(const Texture2D& fr, unsigned int fr_level, const Texture2D& to, unsigned int to_level) {

        unsigned int fr_scale = static_cast<unsigned int>(std::pow(2, fr_level));
        unsigned int to_scale = static_cast<unsigned int>(std::pow(2, to_level));

        unsigned int fw = fr.width / fr_scale;
        unsigned int fh = fr.height / fr_scale;
        unsigned int fd = fr.depth;

        unsigned int tw = to.width / to_scale;
        unsigned int th = to.height / to_scale;
        unsigned int td = to.depth;

        if (fw != tw || fh != th || fd != td) {
            //CORE_ERROR("Unable to copy image data, mismatch width, height or depth!");
            return;
        }

        if (fr.target != to.target) {
            //CORE_ERROR("Unable to copy image data, incompatible targets!");
            return;
        }

        glCopyImageSubData(fr.id, fr.target, fr_level, 0, 0, 0, to.id, to.target, to_level, 0, 0, 0, fw, fh, fd);
    }
}