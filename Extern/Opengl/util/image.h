
#pragma once

#include <memory>
#include <string>
#include <glad/glad.h>

namespace utils {

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

}
