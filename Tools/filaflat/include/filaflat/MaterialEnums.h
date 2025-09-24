#pragma once
#include <stddef.h>
#include <stdint.h>

namespace filament {

// update this when a new version of filament wouldn't work with older materials
static constexpr size_t MATERIAL_VERSION = 62;

/**
 * Supported shading models
 */
enum class Shading : uint8_t {
    UNLIT,                  //!< no lighting applied, emissive possible
    LIT,                    //!< default, standard lighting
    SUBSURFACE,             //!< subsurface lighting model
    CLOTH,                  //!< cloth lighting model
    SPECULAR_GLOSSINESS,    //!< legacy lighting model
};

/**
 * Attribute interpolation types in the fragment shader
 */
enum class Interpolation : uint8_t {
    SMOOTH,                 //!< default, smooth interpolation
    FLAT                    //!< flat interpolation
};

/**
 * Shader quality, affect some global quality parameters
 */
enum class ShaderQuality : int8_t {
    DEFAULT = -1,   // LOW on mobile, HIGH on desktop
    LOW     = 0,    // enable optimizations that can slightly affect correctness
    NORMAL  = 1,    // normal quality, correctness honored
    HIGH    = 2     // higher quality (e.g. better upscaling, etc...)
};

/**
 * Supported blending modes
 */
enum class BlendingMode  {
    //! material is opaque
    OPAQUE,
    //! material is transparent and color is alpha-pre-multiplied, affects diffuse lighting only
    TRANSPARENT,
    //! material is additive (e.g.: hologram)
    ADD,
    //! material is masked (i.e. alpha tested)
    MASKED,
    /**
     * material is transparent and color is alpha-pre-multiplied, affects specular lighting
     * when adding more entries, change the size of FRenderer::CommandKey::blending
     */
    FADE,
    //! material darkens what's behind it
    MULTIPLY,
    //! material brightens what's behind it
    SCREEN,
    //! custom blending function
    CUSTOM,
};

/**
 * How transparent objects are handled
 */
enum class TransparencyMode : uint8_t {
    //! the transparent object is drawn honoring the raster state
    DEFAULT,
    /**
     * the transparent object is first drawn in the depth buffer,
     * then in the color buffer, honoring the culling mode, but ignoring the depth test function
     */
    TWO_PASSES_ONE_SIDE,

    /**
     * the transparent object is drawn twice in the color buffer,
     * first with back faces only, then with front faces; the culling
     * mode is ignored. Can be combined with two-sided lighting
     */
    TWO_PASSES_TWO_SIDES
};



/**
 * Material domains
 */
enum class MaterialDomain : uint8_t {
    SURFACE         = 0, //!< shaders applied to renderables
    POST_PROCESS    = 1, //!< shaders applied to rendered buffers
    COMPUTE         = 2, //!< compute shader
};

/**
 * Specular occlusion
 */
enum class SpecularAmbientOcclusion : uint8_t {
    NONE            = 0, //!< no specular occlusion
    SIMPLE          = 1, //!< simple specular occlusion
    BENT_NORMALS    = 2, //!< more accurate specular occlusion, requires bent normals
};

/**
 * Refraction
 */
enum class RefractionMode : uint8_t {
    NONE            = 0, //!< no refraction
    CUBEMAP         = 1, //!< refracted rays go to the ibl cubemap
    SCREEN_SPACE    = 2, //!< refracted rays go to screen space
};

/**
 * Refraction type
 */
enum class RefractionType : uint8_t {
    SOLID           = 0, //!< refraction through solid objects (e.g. a sphere)
    THIN            = 1, //!< refraction through thin objects (e.g. window)
};

/**
 * Reflection mode
 */
enum class ReflectionMode : uint8_t {
    DEFAULT         = 0, //! reflections sample from the scene's IBL only
    SCREEN_SPACE    = 1, //! reflections sample from screen space, and fallback to the scene's IBL
};



static constexpr size_t MATERIAL_PROPERTIES_COUNT = 30;
enum class Property : uint8_t {
    BASE_COLOR,              //!< float4, all shading models
    ROUGHNESS,               //!< float,  lit shading models only
    METALLIC,                //!< float,  all shading models, except unlit and cloth
    REFLECTANCE,             //!< float,  all shading models, except unlit and cloth
    AMBIENT_OCCLUSION,       //!< float,  lit shading models only, except subsurface and cloth
    CLEAR_COAT,              //!< float,  lit shading models only, except subsurface and cloth
    CLEAR_COAT_ROUGHNESS,    //!< float,  lit shading models only, except subsurface and cloth
    CLEAR_COAT_NORMAL,       //!< float,  lit shading models only, except subsurface and cloth
    ANISOTROPY,              //!< float,  lit shading models only, except subsurface and cloth
    ANISOTROPY_DIRECTION,    //!< float3, lit shading models only, except subsurface and cloth
    THICKNESS,               //!< float,  subsurface shading model only
    SUBSURFACE_POWER,        //!< float,  subsurface shading model only
    SUBSURFACE_COLOR,        //!< float3, subsurface and cloth shading models only
    SHEEN_COLOR,             //!< float3, lit shading models only, except subsurface
    SHEEN_ROUGHNESS,         //!< float3, lit shading models only, except subsurface and cloth
    SPECULAR_COLOR,          //!< float3, specular-glossiness shading model only
    GLOSSINESS,              //!< float,  specular-glossiness shading model only
    EMISSIVE,                //!< float4, all shading models
    NORMAL,                  //!< float3, all shading models only, except unlit
    POST_LIGHTING_COLOR,     //!< float4, all shading models
    POST_LIGHTING_MIX_FACTOR,//!< float, all shading models
    CLIP_SPACE_TRANSFORM,    //!< mat4,   vertex shader only
    ABSORPTION,              //!< float3, how much light is absorbed by the material
    TRANSMISSION,            //!< float,  how much light is refracted through the material
    IOR,                     //!< float,  material's index of refraction
    MICRO_THICKNESS,         //!< float, thickness of the thin layer
    BENT_NORMAL,             //!< float3, all shading models only, except unlit
    SPECULAR_FACTOR,         //!< float, lit shading models only, except subsurface and cloth
    SPECULAR_COLOR_FACTOR,   //!< float3, lit shading models only, except subsurface and cloth
    SHADOW_STRENGTH,         //!< float, [0, 1] strength of shadows received by this material

    // when adding new Properties, make sure to update MATERIAL_PROPERTIES_COUNT
};



} // namespace filament
