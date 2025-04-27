#pragma once
#include<string>

namespace Rendering::Resources
{
    class Texture2D {
    public:
        unsigned int id;
        unsigned int target;
        unsigned int format, i_format;  // internal format
        void SetSampleState() const;

    public:
        unsigned int width, height, depth;
        unsigned int n_levels;

        Texture2D(const std::string& img_path, unsigned int levels = 0);
        Texture2D(const std::string& img_path, unsigned int resolution, unsigned int levels);
        Texture2D(const std::string& directory, const std::string& extension, unsigned int resolution, unsigned int levels);
        Texture2D(unsigned int target, unsigned int width, unsigned int height, unsigned int depth, unsigned int i_format, unsigned int levels);
        ~Texture2D();

        Texture2D(const Texture2D&) = delete;
        Texture2D& operator=(const Texture2D&) = delete;
        Texture2D(Texture2D&& other) noexcept = default;
        Texture2D& operator=(Texture2D&& other) noexcept = default;
        void Resize(unsigned int width, unsigned int height);
        void Bind(unsigned int index) const ;
        void Unbind(unsigned int index) const;
        void BindILS(unsigned int level, unsigned int index, unsigned int access) const;
        void UnbindILS(unsigned int index) const;

        void GenerateMipmap() const;
        void Clear(unsigned int level) const;
        void Invalidate(unsigned int level) const;

        static void Copy(const Texture2D& fr, unsigned int fr_level, const Texture2D& to, unsigned int to_level);
    };
}