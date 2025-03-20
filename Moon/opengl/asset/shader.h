
#pragma once

#include <string>
#include <vector>
#include <glad/glad.h>
#include "asset.h"

namespace asset {

    class Shader : public IAsset {
      protected:
        std::string source_path;
        std::string source_code;
        std::vector<GLuint> shaders;

        void ReadShader(const std::string& path, const std::string& macro, std::string& output);
        void LoadShader(GLenum type);
        void LinkShaders();

      public:  // rule of five
        Shader() : IAsset() {}
        Shader(const std::string& source_path);
        Shader(const std::string& binary_path, GLenum format);
        virtual ~Shader();

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;
        Shader(Shader&& other) = default;
        Shader& operator=(Shader&& other) = default;

      public:
        void LoadFromSource(const std::string& source);
        void Bind() const override;
        void Unbind() const override;

        void Save() const;
        void Inspect() const;

        template<typename T>
        void SetUniform(GLuint location, const T& val) const;

        template<typename T>
        void SetUniformArray(GLuint location, GLuint size, const std::vector<T>& vec) const;
    };
    
    class CShader : public Shader {
      private:
        GLint local_size_x, local_size_y, local_size_z;

      public:  // rule of zero
        CShader(const std::string& source_path);
        CShader(const std::string& binary_path, GLenum format);
        CShader(const std::string& source_code,const std::string& source_path);

      public:
        void Dispatch(GLuint nx, GLuint ny, GLuint nz = 1) const;
        void SyncWait(GLbitfield barriers = GL_SHADER_STORAGE_BARRIER_BIT) const;
    };

}