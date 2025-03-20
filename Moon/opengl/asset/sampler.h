
#pragma once

#include "asset.h"

namespace asset {

    enum class FilterMode {
        Point, Bilinear, Trilinear  // preset samplers
    };

    class Sampler : public IAsset {
      public:
        Sampler(FilterMode mode = FilterMode::Point);
        ~Sampler();

        Sampler(const Sampler&) = delete;
        Sampler& operator=(const Sampler&) = delete;
        Sampler(Sampler&& other) noexcept = default;
        Sampler& operator=(Sampler&& other) noexcept = default;

      public:
        void Bind(GLuint index) const override;
        void Unbind(GLuint index) const override;

        template<typename T>
        void SetParam(GLenum name, T value) const;

        template<typename T>
        void SetParam(GLenum name, const T* value) const;
    };

}