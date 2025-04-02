#pragma once

#include <vector>
#include <glad/glad.h>

namespace asset {
    /*
   - VBO  (Vertex Buffer Object)
   - IBO  (Index Buffer Object)
   - PBO  (Pixel Buffer Object)
   - ATC  (Atomic Counters)
   - UBO  (Uniform Buffer Object)
   - SSBO (Shader Storage Buffer Object)
    */

    class IBuffer {
      protected:
        GLuint id;
        GLsizeiptr size;
        mutable void* data_ptr;

        IBuffer();
        IBuffer(GLsizeiptr size, const void* data, GLbitfield access);
        virtual ~IBuffer();

        IBuffer(const IBuffer&) = delete;
        IBuffer& operator=(const IBuffer&) = delete;
        IBuffer(IBuffer&& other) noexcept;
        IBuffer& operator=(IBuffer&& other) noexcept;

      public:
        GLuint ID() const { return this->id; }
        GLsizeiptr Size() const { return this->size; }
        void* const Data() const { return this->data_ptr; }

        static void Copy(GLuint fr, GLuint to, GLintptr fr_offset, GLintptr to_offset, GLsizeiptr size);

        void GetData(void* data) const;
        void GetData(GLintptr offset, GLsizeiptr size, void* data) const;
        void SetData(const void* data) const;
        void SetData(GLintptr offset, GLsizeiptr size, const void* data) const;

        void Acquire(GLbitfield access) const;
        void Release() const;
        void Clear() const;
        void Clear(GLintptr offset, GLsizeiptr size) const;
        void Flush() const;
        void Flush(GLintptr offset, GLsizeiptr size) const;
        void Invalidate() const;
        void Invalidate(GLintptr offset, GLsizeiptr size) const;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////

    class VBO : public IBuffer {
      public:
        VBO(GLsizeiptr size, const void* data, GLbitfield access = 0) : IBuffer(size, data, access) {}
    };

    class IBO : public IBuffer {
      public:
        IBO(GLsizeiptr size, const void* data, GLbitfield access = 0) : IBuffer(size, data, access) {}
    };
    //像素缓冲对象
    class PBO : public IBuffer {
      public:
        PBO(GLsizeiptr size, const void* data, GLbitfield access = 0) : IBuffer(size, data, access) {}
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////

    class IIndexedBuffer : public IBuffer {
      protected:
        GLuint index;
        GLenum target;
        IIndexedBuffer() : IBuffer(), index(0), target(0) {}
        IIndexedBuffer(GLuint index, GLsizeiptr size, GLbitfield access) : IBuffer(size, NULL, access), index(index) {}

      public:
        void Reset(GLuint index);
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////

    class ATC : public IIndexedBuffer {
      public:
        ATC() = default;
        ATC(GLuint index, GLsizeiptr size, GLbitfield access = GL_DYNAMIC_STORAGE_BIT);
    };

    class SSBO : public IIndexedBuffer {
      public:
        SSBO() = default;
        SSBO(GLuint index, GLsizeiptr size, GLbitfield access = GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT);
    };

    class UBO : public IIndexedBuffer {
      private:
        using u_vec = std::vector<GLuint>;
        u_vec offset_vec;  // each uniform's aligned byte offset
        u_vec stride_vec;  // each uniform's byte stride (with padding)
        u_vec length_vec;  // each uniform's byte length (w/o. padding)

      public:
        UBO() = default;
        UBO(GLuint index, const u_vec& offset, const u_vec& length, const u_vec& stride);
        UBO(GLuint shader, GLuint block_id, GLbitfield access = GL_DYNAMIC_STORAGE_BIT);
        void SetUniform(GLuint uid, const void* data) const;
        void SetUniform(GLuint fr, GLuint to, const void* data) const;
    };

}