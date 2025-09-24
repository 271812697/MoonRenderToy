#pragma once

#include <vector>
#include <array>
#include <string>


namespace filament
{
namespace backend {



enum class ShaderStage : uint8_t {
    NONE = 0,
    VERTEX = 0x1,
    GEOMERTY = 0x2,
    FRAGMENT = 0x4,
    COMPUTE = 0x8,
};

static constexpr size_t PIPELINE_STAGE_COUNT = 2;
enum class ShaderStageFlags : uint8_t {
    NONE        =    0,
    VERTEX      =    0x1,
    GEOMERTY    =    0x2,
    FRAGMENT    =    0x4,
    COMPUTE     =    0x8,
    ALL_SHADER_STAGE_FLAGS = VERTEX | GEOMERTY | FRAGMENT | COMPUTE
};

constexpr bool hasShaderType(ShaderStageFlags flags, ShaderStage type) noexcept {
    switch (type) {
        case ShaderStage::VERTEX:
            return bool(uint8_t(flags) & uint8_t(ShaderStageFlags::VERTEX));
        case ShaderStage::GEOMERTY:
            return bool(uint8_t(flags) & uint8_t(ShaderStageFlags::GEOMERTY));
        case ShaderStage::FRAGMENT:
            return bool(uint8_t(flags) & uint8_t(ShaderStageFlags::FRAGMENT));
        case ShaderStage::COMPUTE:
            return bool(uint8_t(flags) & uint8_t(ShaderStageFlags::COMPUTE));
    }
}

enum class TextureType : uint8_t {
    FLOAT,
    INT,
    UINT,
    DEPTH,
    STENCIL,
    DEPTH_STENCIL
};

 inline std::string to_string(TextureType type) noexcept {
    switch (type) {
        case TextureType::FLOAT:            return "FLOAT";
        case TextureType::INT:              return "INT";
        case TextureType::UINT:             return "UINT";
        case TextureType::DEPTH:            return "DEPTH";
        case TextureType::STENCIL:          return "STENCIL";
        case TextureType::DEPTH_STENCIL:    return "DEPTH_STENCIL";
    }
    return "UNKNOWN";
}

 enum class DescriptorType : uint8_t {
     SAMPLER_2D_FLOAT,
     SAMPLER_2D_INT,
     SAMPLER_2D_UINT,
     SAMPLER_2D_DEPTH,

     SAMPLER_2D_ARRAY_FLOAT,
     SAMPLER_2D_ARRAY_INT,
     SAMPLER_2D_ARRAY_UINT,
     SAMPLER_2D_ARRAY_DEPTH,

     SAMPLER_CUBE_FLOAT,
     SAMPLER_CUBE_INT,
     SAMPLER_CUBE_UINT,
     SAMPLER_CUBE_DEPTH,

     SAMPLER_CUBE_ARRAY_FLOAT,
     SAMPLER_CUBE_ARRAY_INT,
     SAMPLER_CUBE_ARRAY_UINT,
     SAMPLER_CUBE_ARRAY_DEPTH,

     SAMPLER_3D_FLOAT,
     SAMPLER_3D_INT,
     SAMPLER_3D_UINT,

     SAMPLER_2D_MS_FLOAT,
     SAMPLER_2D_MS_INT,
     SAMPLER_2D_MS_UINT,

     SAMPLER_2D_MS_ARRAY_FLOAT,
     SAMPLER_2D_MS_ARRAY_INT,
     SAMPLER_2D_MS_ARRAY_UINT,
     SAMPLER_BUFFERINT,
     SAMPLER_BUFFERFLOAT,
     SAMPLER_BUFFERUINT,
     SAMPLER_EXTERNAL,
     UNIFORM_BUFFER,
     SHADER_STORAGE_BUFFER,
     INPUT_ATTACHMENT,
 };

constexpr bool isDepthDescriptor(DescriptorType const type) noexcept {
    switch (type) {
        case DescriptorType::SAMPLER_2D_DEPTH:
        case DescriptorType::SAMPLER_2D_ARRAY_DEPTH:
        case DescriptorType::SAMPLER_CUBE_DEPTH:
        case DescriptorType::SAMPLER_CUBE_ARRAY_DEPTH:
            return true;
        default: ;
    }
    return false;
}

constexpr bool isFloatDescriptor(DescriptorType const type) noexcept {
    switch (type) {
        case DescriptorType::SAMPLER_2D_FLOAT:
        case DescriptorType::SAMPLER_2D_ARRAY_FLOAT:
        case DescriptorType::SAMPLER_CUBE_FLOAT:
        case DescriptorType::SAMPLER_CUBE_ARRAY_FLOAT:
        case DescriptorType::SAMPLER_3D_FLOAT:
        case DescriptorType::SAMPLER_2D_MS_FLOAT:
        case DescriptorType::SAMPLER_2D_MS_ARRAY_FLOAT:
            return true;
        default: ;
    }
    return false;
}

constexpr bool isIntDescriptor(DescriptorType const type) noexcept {
    switch (type) {
        case DescriptorType::SAMPLER_2D_INT:
        case DescriptorType::SAMPLER_2D_ARRAY_INT:
        case DescriptorType::SAMPLER_CUBE_INT:
        case DescriptorType::SAMPLER_CUBE_ARRAY_INT:
        case DescriptorType::SAMPLER_3D_INT:
        case DescriptorType::SAMPLER_2D_MS_INT:
        case DescriptorType::SAMPLER_2D_MS_ARRAY_INT:
            return true;
        default: ;
    }
    return false;
}

constexpr bool isUnsignedIntDescriptor(DescriptorType const type) noexcept {
    switch (type) {
        case DescriptorType::SAMPLER_2D_UINT:
        case DescriptorType::SAMPLER_2D_ARRAY_UINT:
        case DescriptorType::SAMPLER_CUBE_UINT:
        case DescriptorType::SAMPLER_CUBE_ARRAY_UINT:
        case DescriptorType::SAMPLER_3D_UINT:
        case DescriptorType::SAMPLER_2D_MS_UINT:
        case DescriptorType::SAMPLER_2D_MS_ARRAY_UINT:
            return true;
        default: ;
    }
    return false;
}

constexpr bool is3dTypeDescriptor(DescriptorType const type) noexcept {
    switch (type) {
        case DescriptorType::SAMPLER_3D_FLOAT:
        case DescriptorType::SAMPLER_3D_INT:
        case DescriptorType::SAMPLER_3D_UINT:
            return true;
        default: ;
    }
    return false;
}

constexpr bool is2dTypeDescriptor(DescriptorType const type) noexcept {
    switch (type) {
        case DescriptorType::SAMPLER_2D_FLOAT:
        case DescriptorType::SAMPLER_2D_INT:
        case DescriptorType::SAMPLER_2D_UINT:
        case DescriptorType::SAMPLER_2D_DEPTH:
        case DescriptorType::SAMPLER_2D_MS_FLOAT:
        case DescriptorType::SAMPLER_2D_MS_INT:
        case DescriptorType::SAMPLER_2D_MS_UINT:
            return true;
        default: ;
    }
    return false;
}

constexpr bool is2dArrayTypeDescriptor(DescriptorType const type) noexcept {
    switch (type) {
        case DescriptorType::SAMPLER_2D_ARRAY_FLOAT:
        case DescriptorType::SAMPLER_2D_ARRAY_INT:
        case DescriptorType::SAMPLER_2D_ARRAY_UINT:
        case DescriptorType::SAMPLER_2D_ARRAY_DEPTH:
        case DescriptorType::SAMPLER_2D_MS_ARRAY_FLOAT:
        case DescriptorType::SAMPLER_2D_MS_ARRAY_INT:
        case DescriptorType::SAMPLER_2D_MS_ARRAY_UINT:
            return true;
        default: ;
    }
    return false;
}

constexpr bool isCubeTypeDescriptor(DescriptorType const type) noexcept {
    switch (type) {
        case DescriptorType::SAMPLER_CUBE_FLOAT:
        case DescriptorType::SAMPLER_CUBE_INT:
        case DescriptorType::SAMPLER_CUBE_UINT:
        case DescriptorType::SAMPLER_CUBE_DEPTH:
            return true;
        default: ;
    }
    return false;
}

constexpr bool isCubeArrayTypeDescriptor(DescriptorType const type) noexcept {
    switch (type) {
        case DescriptorType::SAMPLER_CUBE_ARRAY_FLOAT:
        case DescriptorType::SAMPLER_CUBE_ARRAY_INT:
        case DescriptorType::SAMPLER_CUBE_ARRAY_UINT:
        case DescriptorType::SAMPLER_CUBE_ARRAY_DEPTH:
            return true;
        default: ;
    }
    return false;
}


 inline std::string to_string(DescriptorType type) noexcept {
    #define DESCRIPTOR_TYPE_CASE(TYPE)  case DescriptorType::TYPE: return #TYPE;
    switch (type) {
        DESCRIPTOR_TYPE_CASE(SAMPLER_2D_FLOAT)
        DESCRIPTOR_TYPE_CASE(SAMPLER_2D_INT)
        DESCRIPTOR_TYPE_CASE(SAMPLER_2D_UINT)
        DESCRIPTOR_TYPE_CASE(SAMPLER_2D_DEPTH)
        DESCRIPTOR_TYPE_CASE(SAMPLER_2D_ARRAY_FLOAT)
        DESCRIPTOR_TYPE_CASE(SAMPLER_2D_ARRAY_INT)
        DESCRIPTOR_TYPE_CASE(SAMPLER_2D_ARRAY_UINT)
        DESCRIPTOR_TYPE_CASE(SAMPLER_2D_ARRAY_DEPTH)
        DESCRIPTOR_TYPE_CASE(SAMPLER_CUBE_FLOAT)
        DESCRIPTOR_TYPE_CASE(SAMPLER_CUBE_INT)
        DESCRIPTOR_TYPE_CASE(SAMPLER_CUBE_UINT)
        DESCRIPTOR_TYPE_CASE(SAMPLER_CUBE_DEPTH)
        DESCRIPTOR_TYPE_CASE(SAMPLER_CUBE_ARRAY_FLOAT)
        DESCRIPTOR_TYPE_CASE(SAMPLER_CUBE_ARRAY_INT)
        DESCRIPTOR_TYPE_CASE(SAMPLER_CUBE_ARRAY_UINT)
        DESCRIPTOR_TYPE_CASE(SAMPLER_CUBE_ARRAY_DEPTH)
        DESCRIPTOR_TYPE_CASE(SAMPLER_3D_FLOAT)
        DESCRIPTOR_TYPE_CASE(SAMPLER_3D_INT)
        DESCRIPTOR_TYPE_CASE(SAMPLER_3D_UINT)
        DESCRIPTOR_TYPE_CASE(SAMPLER_2D_MS_FLOAT)
        DESCRIPTOR_TYPE_CASE(SAMPLER_2D_MS_INT)
        DESCRIPTOR_TYPE_CASE(SAMPLER_2D_MS_UINT)
        DESCRIPTOR_TYPE_CASE(SAMPLER_2D_MS_ARRAY_FLOAT)
        DESCRIPTOR_TYPE_CASE(SAMPLER_2D_MS_ARRAY_INT)
        DESCRIPTOR_TYPE_CASE(SAMPLER_2D_MS_ARRAY_UINT)
        DESCRIPTOR_TYPE_CASE(SAMPLER_EXTERNAL)
        DESCRIPTOR_TYPE_CASE(UNIFORM_BUFFER)
        DESCRIPTOR_TYPE_CASE(SHADER_STORAGE_BUFFER)
        DESCRIPTOR_TYPE_CASE(INPUT_ATTACHMENT)
    }
    return "UNKNOWN";
    #undef DESCRIPTOR_TYPE_CASE
}

enum class DescriptorFlags : uint8_t {
    NONE = 0x00,

    // Indicate a UNIFORM_BUFFER will have dynamic offsets.
    DYNAMIC_OFFSET = 0x01,

    // To indicate a texture/sampler type should be unfiltered.
    UNFILTERABLE = 0x02,
};

using descriptor_set_t = uint8_t;

using descriptor_binding_t = uint8_t;

struct DescriptorSetLayoutBinding {
    static bool isSampler(DescriptorType type) noexcept {
        return int(type) <= int(DescriptorType::SAMPLER_EXTERNAL);
    }
    static bool isBuffer(DescriptorType type) noexcept {
        return type == DescriptorType::UNIFORM_BUFFER ||
               type == DescriptorType::SHADER_STORAGE_BUFFER;
    }
    DescriptorType type;
    ShaderStageFlags stageFlags;
    descriptor_binding_t binding;
    DescriptorFlags flags = DescriptorFlags::NONE;
    uint16_t count = 0;

    friend bool operator==(DescriptorSetLayoutBinding const& lhs,
            DescriptorSetLayoutBinding const& rhs) noexcept {
        return lhs.type == rhs.type &&
               lhs.flags == rhs.flags &&
               lhs.count == rhs.count &&
               lhs.stageFlags == rhs.stageFlags;
    }
};

/**
 * Supported uniform types
 */
enum class UniformType : uint8_t {
    BOOL,
    BOOL2,
    BOOL3,
    BOOL4,
    FLOAT,
    FLOAT2,
    FLOAT3,
    FLOAT4,
    INT,
    INT2,
    INT3,
    INT4,
    UINT,
    UINT2,
    UINT3,
    UINT4,
    MAT3,   //!< a 3x3 float matrix
    MAT4,   //!< a 4x4 float matrix
    STRUCT
};

/**
 * Supported constant parameter types
 */
enum class ConstantType : uint8_t {
  INT,
  FLOAT,
  BOOL
};

enum class Precision : uint8_t {
    LOW,
    MEDIUM,
    HIGH,
    DEFAULT
};

/**
 * Shader compiler priority queue
 */
enum class CompilerPriorityQueue : uint8_t {
    HIGH,
    LOW
};

//! Texture sampler type
enum class SamplerType : uint8_t {
    SAMPLER_2D,             //!< 2D texture
    SAMPLER_2D_ARRAY,       //!< 2D array texture
    SAMPLER_CUBEMAP,        //!< Cube map texture
    SAMPLER_EXTERNAL,       //!< External texture
    SAMPLER_3D,             //!< 3D texture
    SAMPLER_CUBEMAP_ARRAY,  //!< Cube map array texture (feature level 2)
    SAMPLER_BUFFER
};

 inline std::string to_string(SamplerType const type) noexcept {
    switch (type) {
        case SamplerType::SAMPLER_2D:
            return "SAMPLER_2D";
        case SamplerType::SAMPLER_2D_ARRAY:
            return "SAMPLER_2D_ARRAY";
        case SamplerType::SAMPLER_CUBEMAP:
            return "SAMPLER_CUBEMAP";
        case SamplerType::SAMPLER_EXTERNAL:
            return "SAMPLER_EXTERNAL";
        case SamplerType::SAMPLER_3D:
            return "SAMPLER_3D";
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
            return "SAMPLER_CUBEMAP_ARRAY";
    }
    return "Unknown";
}

//! Subpass type
enum class SubpassType : uint8_t {
    SUBPASS_INPUT
};

//! Texture sampler format
enum class SamplerFormat : uint8_t {
    INT = 0,        //!< signed integer sampler
    UINT = 1,       //!< unsigned integer sampler
    FLOAT = 2,      //!< float sampler
    SHADOW = 3      //!< shadow sampler (PCF)
};

 inline std::string to_string(SamplerFormat const format) noexcept {
    switch (format) {
        case SamplerFormat::INT:
            return "INT";
        case SamplerFormat::UINT:
            return "UINT";
        case SamplerFormat::FLOAT:
            return "FLOAT";
        case SamplerFormat::SHADOW:
            return "SHADOW";
    }
    return "Unknown";
}

/**
 * Supported element types
 */
enum class ElementType : uint8_t {
    BYTE,
    BYTE2,
    BYTE3,
    BYTE4,
    UBYTE,
    UBYTE2,
    UBYTE3,
    UBYTE4,
    SHORT,
    SHORT2,
    SHORT3,
    SHORT4,
    USHORT,
    USHORT2,
    USHORT3,
    USHORT4,
    INT,
    UINT,
    FLOAT,
    FLOAT2,
    FLOAT3,
    FLOAT4,
    HALF,
    HALF2,
    HALF3,
    HALF4,
};

//! Buffer object binding type
enum class BufferObjectBinding : uint8_t {
    VERTEX,
    UNIFORM,
    SHADER_STORAGE
};

inline std::string to_string(BufferObjectBinding type) noexcept {
    switch (type) {
        case BufferObjectBinding::VERTEX:           return "VERTEX";
        case BufferObjectBinding::UNIFORM:          return "UNIFORM";
        case BufferObjectBinding::SHADER_STORAGE:   return "SHADER_STORAGE";
    }
    return "UNKNOWN";
}

//! Face culling Mode
enum class CullingMode : uint8_t {
    NONE,               //!< No culling, front and back faces are visible
    FRONT,              //!< Front face culling, only back faces are visible
    BACK,               //!< Back face culling, only front faces are visible
    FRONT_AND_BACK      //!< Front and Back, geometry is not visible
};

//! Pixel Data Format
enum class PixelDataFormat : uint8_t {
    R,                  //!< One Red channel, float
    R_INTEGER,          //!< One Red channel, integer
    RG,                 //!< Two Red and Green channels, float
    RG_INTEGER,         //!< Two Red and Green channels, integer
    RGB,                //!< Three Red, Green and Blue channels, float
    RGB_INTEGER,        //!< Three Red, Green and Blue channels, integer
    RGBA,               //!< Four Red, Green, Blue and Alpha channels, float
    RGBA_INTEGER,       //!< Four Red, Green, Blue and Alpha channels, integer
    UNUSED,             // used to be rgbm
    DEPTH_COMPONENT,    //!< Depth, 16-bit or 24-bits usually
    DEPTH_STENCIL,      //!< Two Depth (24-bits) + Stencil (8-bits) channels
    ALPHA               //! One Alpha channel, float
};

//! Pixel Data Type
enum class PixelDataType : uint8_t {
    UBYTE,                //!< unsigned byte
    BYTE,                 //!< signed byte
    USHORT,               //!< unsigned short (16-bit)
    SHORT,                //!< signed short (16-bit)
    UINT,                 //!< unsigned int (32-bit)
    INT,                  //!< signed int (32-bit)
    HALF,                 //!< half-float (16-bit float)
    FLOAT,                //!< float (32-bits float)
    COMPRESSED,           //!< compressed pixels, @see CompressedPixelDataType
    UINT_10F_11F_11F_REV, //!< three low precision floating-point numbers
    USHORT_565,           //!< unsigned int (16-bit), encodes 3 RGB channels
    UINT_2_10_10_10_REV,  //!< unsigned normalized 10 bits RGB, 2 bits alpha
};

//! Compressed pixel data types
enum class CompressedPixelDataType : uint16_t {
    // Mandatory in GLES 3.0 and GL 4.3
    EAC_R11, EAC_R11_SIGNED, EAC_RG11, EAC_RG11_SIGNED,
    ETC2_RGB8, ETC2_SRGB8,
    ETC2_RGB8_A1, ETC2_SRGB8_A1,
    ETC2_EAC_RGBA8, ETC2_EAC_SRGBA8,

    // Available everywhere except Android/iOS
    DXT1_RGB, DXT1_RGBA, DXT3_RGBA, DXT5_RGBA,
    DXT1_SRGB, DXT1_SRGBA, DXT3_SRGBA, DXT5_SRGBA,

    // ASTC formats are available with a GLES extension
    RGBA_ASTC_4x4,
    RGBA_ASTC_5x4,
    RGBA_ASTC_5x5,
    RGBA_ASTC_6x5,
    RGBA_ASTC_6x6,
    RGBA_ASTC_8x5,
    RGBA_ASTC_8x6,
    RGBA_ASTC_8x8,
    RGBA_ASTC_10x5,
    RGBA_ASTC_10x6,
    RGBA_ASTC_10x8,
    RGBA_ASTC_10x10,
    RGBA_ASTC_12x10,
    RGBA_ASTC_12x12,
    SRGB8_ALPHA8_ASTC_4x4,
    SRGB8_ALPHA8_ASTC_5x4,
    SRGB8_ALPHA8_ASTC_5x5,
    SRGB8_ALPHA8_ASTC_6x5,
    SRGB8_ALPHA8_ASTC_6x6,
    SRGB8_ALPHA8_ASTC_8x5,
    SRGB8_ALPHA8_ASTC_8x6,
    SRGB8_ALPHA8_ASTC_8x8,
    SRGB8_ALPHA8_ASTC_10x5,
    SRGB8_ALPHA8_ASTC_10x6,
    SRGB8_ALPHA8_ASTC_10x8,
    SRGB8_ALPHA8_ASTC_10x10,
    SRGB8_ALPHA8_ASTC_12x10,
    SRGB8_ALPHA8_ASTC_12x12,

    // RGTC formats available with a GLES extension
    RED_RGTC1,              // BC4 unsigned
    SIGNED_RED_RGTC1,       // BC4 signed
    RED_GREEN_RGTC2,        // BC5 unsigned
    SIGNED_RED_GREEN_RGTC2, // BC5 signed

    // BPTC formats available with a GLES extension
    RGB_BPTC_SIGNED_FLOAT,  // BC6H signed
    RGB_BPTC_UNSIGNED_FLOAT,// BC6H unsigned
    RGBA_BPTC_UNORM,        // BC7
    SRGB_ALPHA_BPTC_UNORM,  // BC7 sRGB
};

/** Supported texel formats
 * These formats are typically used to specify a texture's internal storage format.
 *
 * Enumerants syntax format
 * ========================
 *
 * `[components][size][type]`
 *
 * `components` : List of stored components by this format.\n
 * `size`       : Size in bit of each component.\n
 * `type`       : Type this format is stored as.\n
 *
 *
 * Name     | Component
 * :--------|:-------------------------------
 * R        | Linear Red
 * RG       | Linear Red, Green
 * RGB      | Linear Red, Green, Blue
 * RGBA     | Linear Red, Green Blue, Alpha
 * SRGB     | sRGB encoded Red, Green, Blue
 * DEPTH    | Depth
 * STENCIL  | Stencil
 *
 * \n
 * Name     | Type
 * :--------|:---------------------------------------------------
 * (none)   | Unsigned Normalized Integer [0, 1]
 * _SNORM   | Signed Normalized Integer [-1, 1]
 * UI       | Unsigned Integer @f$ [0, 2^{size}] @f$
 * I        | Signed Integer @f$ [-2^{size-1}, 2^{size-1}-1] @f$
 * F        | Floating-point
 *
 *
 * Special color formats
 * ---------------------
 *
 * There are a few special color formats that don't follow the convention above:
 *
 * Name             | Format
 * :----------------|:--------------------------------------------------------------------------
 * RGB565           |  5-bits for R and B, 6-bits for G.
 * RGB5_A1          |  5-bits for R, G and B, 1-bit for A.
 * RGB10_A2         | 10-bits for R, G and B, 2-bits for A.
 * RGB9_E5          | **Unsigned** floating point. 9-bits mantissa for RGB, 5-bits shared exponent
 * R11F_G11F_B10F   | **Unsigned** floating point. 6-bits mantissa, for R and G, 5-bits for B. 5-bits exponent.
 * SRGB8_A8         | sRGB 8-bits with linear 8-bits alpha.
 * DEPTH24_STENCIL8 | 24-bits unsigned normalized integer depth, 8-bits stencil.
 * DEPTH32F_STENCIL8| 32-bits floating-point depth, 8-bits stencil.
 *
 *
 * Compressed texture formats
 * --------------------------
 *
 * Many compressed texture formats are supported as well, which include (but are not limited to)
 * the following list:
 *
 * Name             | Format
 * :----------------|:--------------------------------------------------------------------------
 * EAC_R11          | Compresses R11UI
 * EAC_R11_SIGNED   | Compresses R11I
 * EAC_RG11         | Compresses RG11UI
 * EAC_RG11_SIGNED  | Compresses RG11I
 * ETC2_RGB8        | Compresses RGB8
 * ETC2_SRGB8       | compresses SRGB8
 * ETC2_EAC_RGBA8   | Compresses RGBA8
 * ETC2_EAC_SRGBA8  | Compresses SRGB8_A8
 * ETC2_RGB8_A1     | Compresses RGB8 with 1-bit alpha
 * ETC2_SRGB8_A1    | Compresses sRGB8 with 1-bit alpha
 *
 *
 * @see Texture
 */
enum class TextureFormat : uint16_t {
    // 8-bits per element
    R8, R8_SNORM, R8UI, R8I, STENCIL8,

    // 16-bits per element
    R16F, R16UI, R16I,
    RG8, RG8_SNORM, RG8UI, RG8I,
    RGB565,
    RGB9_E5, // 9995 is actually 32 bpp but it's here for historical reasons.
    RGB5_A1,
    RGBA4,
    DEPTH16,

    // 24-bits per element
    RGB8, SRGB8, RGB8_SNORM, RGB8UI, RGB8I,
    DEPTH24,

    // 32-bits per element
    R32F, R32UI, R32I,
    RG16F, RG16UI, RG16I,
    R11F_G11F_B10F,
    RGBA8, SRGB8_A8,RGBA8_SNORM,
    UNUSED, // used to be rgbm
    RGB10_A2, RGBA8UI, RGBA8I,
    DEPTH32F, DEPTH24_STENCIL8, DEPTH32F_STENCIL8,

    // 48-bits per element
    RGB16F, RGB16UI, RGB16I,

    // 64-bits per element
    RG32F, RG32UI, RG32I,
    RGBA16F, RGBA16UI, RGBA16I,

    // 96-bits per element
    RGB32F, RGB32UI, RGB32I,

    // 128-bits per element
    RGBA32F, RGBA32UI, RGBA32I,

    // compressed formats

    // Mandatory in GLES 3.0 and GL 4.3
    EAC_R11, EAC_R11_SIGNED, EAC_RG11, EAC_RG11_SIGNED,
    ETC2_RGB8, ETC2_SRGB8,
    ETC2_RGB8_A1, ETC2_SRGB8_A1,
    ETC2_EAC_RGBA8, ETC2_EAC_SRGBA8,

    // Available everywhere except Android/iOS
    DXT1_RGB, DXT1_RGBA, DXT3_RGBA, DXT5_RGBA,
    DXT1_SRGB, DXT1_SRGBA, DXT3_SRGBA, DXT5_SRGBA,

    // ASTC formats are available with a GLES extension
    RGBA_ASTC_4x4,
    RGBA_ASTC_5x4,
    RGBA_ASTC_5x5,
    RGBA_ASTC_6x5,
    RGBA_ASTC_6x6,
    RGBA_ASTC_8x5,
    RGBA_ASTC_8x6,
    RGBA_ASTC_8x8,
    RGBA_ASTC_10x5,
    RGBA_ASTC_10x6,
    RGBA_ASTC_10x8,
    RGBA_ASTC_10x10,
    RGBA_ASTC_12x10,
    RGBA_ASTC_12x12,
    SRGB8_ALPHA8_ASTC_4x4,
    SRGB8_ALPHA8_ASTC_5x4,
    SRGB8_ALPHA8_ASTC_5x5,
    SRGB8_ALPHA8_ASTC_6x5,
    SRGB8_ALPHA8_ASTC_6x6,
    SRGB8_ALPHA8_ASTC_8x5,
    SRGB8_ALPHA8_ASTC_8x6,
    SRGB8_ALPHA8_ASTC_8x8,
    SRGB8_ALPHA8_ASTC_10x5,
    SRGB8_ALPHA8_ASTC_10x6,
    SRGB8_ALPHA8_ASTC_10x8,
    SRGB8_ALPHA8_ASTC_10x10,
    SRGB8_ALPHA8_ASTC_12x10,
    SRGB8_ALPHA8_ASTC_12x12,

    // RGTC formats available with a GLES extension
    RED_RGTC1,              // BC4 unsigned
    SIGNED_RED_RGTC1,       // BC4 signed
    RED_GREEN_RGTC2,        // BC5 unsigned
    SIGNED_RED_GREEN_RGTC2, // BC5 signed

    // BPTC formats available with a GLES extension
    RGB_BPTC_SIGNED_FLOAT,  // BC6H signed
    RGB_BPTC_UNSIGNED_FLOAT,// BC6H unsigned
    RGBA_BPTC_UNORM,        // BC7
    SRGB_ALPHA_BPTC_UNORM,  // BC7 sRGB
};

TextureType getTextureType(TextureFormat format) noexcept;

//! Bitmask describing the intended Texture Usage
enum class TextureUsage : uint16_t {
    NONE                = 0x0000,
    COLOR_ATTACHMENT    = 0x0001,            //!< Texture can be used as a color attachment
    DEPTH_ATTACHMENT    = 0x0002,            //!< Texture can be used as a depth attachment
    STENCIL_ATTACHMENT  = 0x0004,            //!< Texture can be used as a stencil attachment
    UPLOADABLE          = 0x0008,            //!< Data can be uploaded into this texture (default)
    SAMPLEABLE          = 0x0010,            //!< Texture can be sampled (default)
    SUBPASS_INPUT       = 0x0020,            //!< Texture can be used as a subpass input
    BLIT_SRC            = 0x0040,            //!< Texture can be used the source of a blit()
    BLIT_DST            = 0x0080,            //!< Texture can be used the destination of a blit()
    PROTECTED           = 0x0100,            //!< Texture can be used for protected content
    DEFAULT             = UPLOADABLE | SAMPLEABLE,   //!< Default texture usage
    ALL_ATTACHMENTS     = COLOR_ATTACHMENT | DEPTH_ATTACHMENT | STENCIL_ATTACHMENT | SUBPASS_INPUT,   //!< Mask of all attachments
};

//! Texture swizzle
enum class TextureSwizzle : uint8_t {
    SUBSTITUTE_ZERO,
    SUBSTITUTE_ONE,
    CHANNEL_0,
    CHANNEL_1,
    CHANNEL_2,
    CHANNEL_3
};

//! returns whether this format a depth format
constexpr bool isDepthFormat(TextureFormat format) noexcept {
    switch (format) {
        case TextureFormat::DEPTH32F:
        case TextureFormat::DEPTH24:
        case TextureFormat::DEPTH16:
        case TextureFormat::DEPTH32F_STENCIL8:
        case TextureFormat::DEPTH24_STENCIL8:
            return true;
        default:
            return false;
    }
}

constexpr bool isStencilFormat(TextureFormat format) noexcept {
    switch (format) {
        case TextureFormat::STENCIL8:
        case TextureFormat::DEPTH24_STENCIL8:
        case TextureFormat::DEPTH32F_STENCIL8:
            return true;
        default:
            return false;
    }
}

constexpr bool isColorFormat(TextureFormat format) noexcept {
    switch (format) {
        // Standard color formats
        case TextureFormat::R8:
        case TextureFormat::RG8:
        case TextureFormat::RGBA8:
        case TextureFormat::R16F:
        case TextureFormat::RG16F:
        case TextureFormat::RGBA16F:
        case TextureFormat::R32F:
        case TextureFormat::RG32F:
        case TextureFormat::RGBA32F:
        case TextureFormat::RGB10_A2:
        case TextureFormat::R11F_G11F_B10F:
        case TextureFormat::SRGB8:
        case TextureFormat::SRGB8_A8:
        case TextureFormat::RGB8:
        case TextureFormat::RGB565:
        case TextureFormat::RGB5_A1:
        case TextureFormat::RGBA4:
            return true;
        default:
            break;
    }
    return false;
}

constexpr bool isUnsignedIntFormat(TextureFormat format) {
    switch (format) {
        case TextureFormat::R8UI:
        case TextureFormat::R16UI:
        case TextureFormat::R32UI:
        case TextureFormat::RG8UI:
        case TextureFormat::RG16UI:
        case TextureFormat::RG32UI:
        case TextureFormat::RGB8UI:
        case TextureFormat::RGB16UI:
        case TextureFormat::RGB32UI:
        case TextureFormat::RGBA8UI:
        case TextureFormat::RGBA16UI:
        case TextureFormat::RGBA32UI:
            return true;

        default:
            return false;
    }
}

constexpr bool isSignedIntFormat(TextureFormat format) {
    switch (format) {
        case TextureFormat::R8I:
        case TextureFormat::R16I:
        case TextureFormat::R32I:
        case TextureFormat::RG8I:
        case TextureFormat::RG16I:
        case TextureFormat::RG32I:
        case TextureFormat::RGB8I:
        case TextureFormat::RGB16I:
        case TextureFormat::RGB32I:
        case TextureFormat::RGBA8I:
        case TextureFormat::RGBA16I:
        case TextureFormat::RGBA32I:
            return true;

        default:
            return false;
    }
}

struct DescriptorSetLayout
{
    std::string label;
    std::vector<DescriptorSetLayoutBinding> bindings;
};
//! blending equation function
enum class BlendEquation : uint8_t {
    ADD,                    //!< the fragment is added to the color buffer
    SUBTRACT,               //!< the fragment is subtracted from the color buffer
    REVERSE_SUBTRACT,       //!< the color buffer is subtracted from the fragment
    MIN,                    //!< the min between the fragment and color buffer
    MAX                     //!< the max between the fragment and color buffer
};

//! blending function
enum class BlendFunction : uint8_t {
    ZERO,                   //!< f(src, dst) = 0
    ONE,                    //!< f(src, dst) = 1
    SRC_COLOR,              //!< f(src, dst) = src
    ONE_MINUS_SRC_COLOR,    //!< f(src, dst) = 1-src
    DST_COLOR,              //!< f(src, dst) = dst
    ONE_MINUS_DST_COLOR,    //!< f(src, dst) = 1-dst
    SRC_ALPHA,              //!< f(src, dst) = src.a
    ONE_MINUS_SRC_ALPHA,    //!< f(src, dst) = 1-src.a
    DST_ALPHA,              //!< f(src, dst) = dst.a
    ONE_MINUS_DST_ALPHA,    //!< f(src, dst) = 1-dst.a
    SRC_ALPHA_SATURATE      //!< f(src, dst) = (1,1,1) * min(src.a, 1 - dst.a), 1
};

//! stencil operation
enum class StencilOperation : uint8_t {
    KEEP,                   //!< Keeps the current value.
    ZERO,                   //!< Sets the value to 0.
    REPLACE,                //!< Sets the value to the stencil reference value.
    INCR,                   //!< Increments the current value. Clamps to the maximum representable unsigned value.
    INCR_WRAP,              //!< Increments the current value. Wraps value to zero when incrementing the maximum representable unsigned value.
    DECR,                   //!< Decrements the current value. Clamps to 0.
    DECR_WRAP,              //!< Decrements the current value. Wraps value to the maximum representable unsigned value when decrementing a value of zero.
    INVERT,                 //!< Bitwise inverts the current value.
};

//! stencil faces
enum class StencilFace : uint8_t {
    FRONT               = 0x1,              //!< Update stencil state for front-facing polygons.
    BACK                = 0x2,              //!< Update stencil state for back-facing polygons.
    FRONT_AND_BACK      = FRONT | BACK,     //!< Update stencil state for all polygons.
};

//! Stream for external textures
enum class StreamType {
    NATIVE,     //!< Not synchronized but copy-free. Good for video.
    ACQUIRED,   //!< Synchronized, copy-free, and take a release callback. Good for AR but requires API 26+.
};

//! Releases an ACQUIRED external texture, guaranteed to be called on the application thread.
using StreamCallback = void(*)(void* image, void* user);

struct PolygonOffset {
    float slope = 0;        // factor in GL-speak
    float constant = 0;     // units in GL-speak
};


} // namespace filament::backend
}

