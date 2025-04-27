#shader vertex
#version 460 core
#pragma optimize(off)

/* Global information sent by the engine */
layout (std140) uniform EngineUBO
{
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
};

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 binormal;

/* Information passed to the fragment shader */
layout(location = 0) out _vtx {
    out vec3 _position;
    out vec3 _normal;
    out vec2 _uv;
    out vec2 _uv2;
    out vec3 _tangent;
    out vec3 _binormal;
};

void main()
{
    

    _position = vec3(ubo_Model * vec4(position, 1.0));
    _normal = normalize(vec3(ubo_Model * vec4(normal, 0.0)));
    _uv = uv;
    _uv2 = uv;
    _tangent = normalize(vec3(ubo_Model * vec4(tangent, 0.0)));
    _binormal = normalize(vec3(ubo_Model * vec4(binormal, 0.0)));

    gl_Position = ubo_Projection * ubo_View * ubo_Model * vec4(position, 1.0);

}
#shader geomery
#version 460 core
#pragma optimize(off)
/* Global information sent by the engine */
layout (std140) uniform EngineUBO
{
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
};
layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

layout(location = 0) in _vtx {
    in vec3 _position;
    in vec3 _normal;
    in vec2 _uv;
    in vec2 _uv2;
    in vec3 _tangent;
    in vec3 _binormal;
} vtx[];

layout(location = 0) out _vtx {
    smooth out vec3 _position;
    smooth out vec3 _normal;
    smooth out vec2 _uv;
    smooth out vec2 _uv2;
    smooth out vec3 _tangent;
    smooth out vec3 _binormal;
};

void main() {


    for (uint i = 0; i < 3; i++) {
        _position = vtx[i]._position+vtx[i]._normal*0.03;
        
        _normal   = vtx[i]._normal;
        _uv       = vtx[i]._uv;
        _uv2      = vtx[i]._uv2;
        _tangent  = vtx[i]._tangent;
        _binormal = vtx[i]._binormal;

        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
        gl_Position = ubo_Projection * ubo_View * vec4(_position, 1.0);
        EmitVertex();
        EndPrimitive();
    }

}


#shader fragment
#version 460 core
#pragma optimize(off)
/* Global information sent by the engine */
layout (std140) uniform EngineUBO
{
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
};
/* Light information sent by the engine */
layout(std430, binding = 0) buffer LightSSBO
{
    mat4 ssbo_Lights[];
};
layout(location = 0) in _vtx {
    in vec3 _position;
    in vec3 _normal;
    in vec2 _uv;
    in vec2 _uv2;
    in vec3 _tangent;
    in vec3 _binormal;
};

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 bloom;
layout(location = 0) uniform float ibl_exposure;

// sampler binding points (texture units) 17-19 are reserved for PBR IBL
layout(binding = 0) uniform samplerCube irradiance_map;
layout(binding = 1) uniform samplerCube prefilter_map;
layout(binding = 2) uniform sampler2D BRDF_LUT;
layout(binding = 3) uniform samplerCube shadow_map1[7];

// sampler binding points (texture units) >= 20 are reserved for PBR use
layout(binding = 20) uniform sampler2D albedo_map;
layout(binding = 21) uniform sampler2D normal_map;
layout(binding = 22) uniform sampler2D metallic_map;
layout(binding = 23) uniform sampler2D roughness_map;
layout(binding = 24) uniform sampler2D ao_map;
layout(binding = 30) uniform sampler2D Emission_map;
layout(binding = 26) uniform sampler2D displace_map;
layout(binding = 27) uniform sampler2D opacity_map;
layout(binding = 28) uniform sampler2D light_map;
layout(binding = 29) uniform sampler2D anisotan_map;  // anisotropic tangent map (RGB)


// default-block (loose) uniform locations >= 900 are reserved for PBR use
layout(location = 900) uniform bool sample_albedo;
layout(location = 901) uniform bool sample_normal;
layout(location = 902) uniform bool sample_metallic;
layout(location = 903) uniform bool sample_roughness;
layout(location = 904) uniform bool sample_ao;
layout(location = 910) uniform bool sample_Emission;
layout(location = 906) uniform bool sample_displace;
layout(location = 907) uniform bool sample_opacity;
layout(location = 908) uniform bool sample_lightmap;
layout(location = 909) uniform bool sample_anisotan;


/***** physically-based material input properties *****/

// shared properties
layout(location = 912) uniform vec4  albedo;         // alpha is not pre-multiplied
layout(location = 913) uniform float roughness;      // clamped at 0.045 so that specular highlight is visible
layout(location = 914) uniform float ao;             // 0.0 = occluded, 1.0 = not occluded
layout(location = 915) uniform vec4  Emission;       // optional emissive color
layout(location = 916) uniform vec2  uv_scale;       // texture coordinates tiling factor
layout(location = 928) uniform float alpha_mask;     // alpha threshold below which fragments are discarded

// standard model, mostly opaque but can have simple alpha blending
layout(location = 917) uniform float metalness;      // should be a binary value 0 or 1
layout(location = 918) uniform float specular;       // 4% F0 = 0.5, 2% F0 = 0.35 (water), clamped to [0.35, 1]
layout(location = 919) uniform float anisotropy;     // ~ [-1, 1]
layout(location = 920) uniform vec3  aniso_dir;      // anisotropy defaults to the tangent direction

// refraction model considers only isotropic dielectrics
layout(location = 921) uniform float transmission;   // ratio of diffuse light transmitted through the material
layout(location = 922) uniform float thickness;      // max volume thickness in the direction of the normal
layout(location = 923) uniform float ior;            // air 1.0, plastic/glass 1.5, water 1.33, gemstone 1.6-2.33
layout(location = 924) uniform vec3  transmittance;  // transmittance color as linear RGB, may differ from albedo
layout(location = 925) uniform float tr_distance;    // transmission distance, smaller for denser IOR
layout(location = 931) uniform uint  volume_type;    // refraction varies by the volume's interior geometry

// cloth model considers only single-layer isotropic dielectrics w/o refraction
layout(location = 926) uniform vec3  sheen_color;
layout(location = 927) uniform vec3  subsurface_color;

// (optional) an additive clear coat layer, not applicable to cloth
layout(location = 929) uniform float clearcoat;
layout(location = 930) uniform float clearcoat_roughness;



// a two-digit number indicative of how the pixel should be shaded
// component x encodes the shading model: standard = 1, refraction = 2, cloth = 3
// component y encodes an additive layer: none = 0, clearcoat = 1, sheen = 2
layout(location = 998) uniform uint model_x;
layout(location = 999) uniform uint model_y;

// pixel data definition
struct Pixel {
    vec3 _position;
    vec3 _normal;
    vec2 _uv;
    vec2 _uv2;
    vec3 _tangent;
    vec3 _binormal;
    bool _has_tbn;
    bool _has_uv2;
    vec3  position;
    vec2  uv;
    mat3  TBN;
    vec3  V;
    vec3  N;
    vec3  R;
    vec3  GN;
    vec3  GR;
    float NoV;
    vec4  albedo;
    float roughness;
    float alpha;
    vec3  ao;
    vec4  emission;
    vec3  diffuse_color;
    vec3  F0;
    vec3  DFG;
    vec3  Ec;
    float metalness;
    float specular;
    float anisotropy;
    vec3  aniso_T;
    vec3  aniso_B;
    float clearcoat;
    float clearcoat_roughness;
    float clearcoat_alpha;
    float eta;
    float transmission;
    vec3  absorption;
    float thickness;
    uint  volume;
    vec3  subsurface_color;
};
////////////////////////////////////////////////////////////////////////////////
#define EPS      1e-5
#define PI       3.141592653589793
#define PI2      6.283185307179586
#define INV_PI   0.318309886183791  // 1 over PI
#define HLF_PI   1.570796326794897  // half PI
#define SQRT2    1.414213562373095
#define SQRT3    1.732050807568877
#define SQRT5    2.236067977499789
#define CBRT2    1.259921049894873  // cube root 2
#define CBRT3    1.442249570307408  // cube root 3
#define G_PHI    1.618033988749894  // golden ratio
#define EULER_E  2.718281828459045  // natural exponent e
#define LN2      0.693147180559945
#define LN10     2.302585092994046
#define INV_LN2  1.442695040888963  // 1 over ln2
#define INV_LN10 0.434294481903252  // 1 over ln10

#define clamp01(x) clamp(x, 0.0, 1.0)

////////////////////////////////////////////////////////////////////////////////

// computes the min/max component of a vec2/vec3/vec4
float min2(const vec2 v) { return min(v.x, v.y); }
float max2(const vec2 v) { return max(v.x, v.y); }
float min3(const vec3 v) { return min(min(v.x, v.y), v.z); }
float max3(const vec3 v) { return max(max(v.x, v.y), v.z); }
float min4(const vec4 v) { return min(min3(v.xyz), v.w); }
float max4(const vec4 v) { return max(max3(v.xyz), v.w); }

// logarithm base 10 and logarithm base 2
float log10(float x) { return log(x) * INV_LN10; }
float log2(float x) { return log(x) * INV_LN2; }

// checks if value x is in range (a, b), returns value type to avoid branching
float step3(float a, float x, float b) { return step(a, x) * step(x, b); }
vec2 step3(const vec2 a, const vec2 x, const vec2 b) { return step(a, x) - step(b, x); }
vec3 step3(const vec3 a, const vec3 x, const vec3 b) { return step(a, x) - step(b, x); }
vec4 step3(const vec4 a, const vec4 x, const vec4 b) { return step(a, x) - step(b, x); }

// optimizes lower power functions, removes the implicit `exp/log` call
float pow2(float x) { return x * x; }
float pow3(float x) { return x * x * x; }
float pow4(float x) { return x * x * x * x; }
float pow5(float x) { return x * x * x * x * x; }

// computes the luminance of a linear RGB color, sRGB must be converted to linear first
float luminance(const vec3 linear_rgb) {
    return dot(linear_rgb, vec3(0.2126, 0.7152, 0.0722));
}

vec3 hsl2rgb(float h, float s, float l) {
    vec3 u = mod(h * 6.0 + vec3(0.0, 4.0, 2.0), 6.0);
    vec3 v = abs(u - 3.0) - 1.0;
    return l + s * (clamp01(v) - 0.5) * (1.0 - abs(2.0 * l - 1.0));
}

vec3 hsv2rgb(float h, float s, float v) {
    if (s <= 1e-4) return vec3(v);  // zero saturation = grayscale color

    float x = fract(h) * 6.0;
    float f = fract(x);
    uint  i = uint(x);

    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));

    switch (i) {
        case 0u: return vec3(v, t, p);
        case 1u: return vec3(q, v, p);
        case 2u: return vec3(p, v, t);
        case 3u: return vec3(p, q, v);
        case 4u: return vec3(t, p, v);
        default: return vec3(v, p, q);
    }
}

vec3 hsl2rgb(const vec3 hsl) {
    return hsl2rgb(hsl.x, hsl.y, hsl.z);
}

vec3 hsv2rgb(const vec3 hsv) {
    return hsv2rgb(hsv.x, hsv.y, hsv.z);
}


vec3 rainbow(float hue) {
    return hsv2rgb(hue, 1.0, 1.0);
}

float bounce(float x, float k) {
    return k - abs(k - mod(x, k * 2));
}

vec4 pack(float x) {
    const vec4 bit_shift = vec4(1.0, 255.0, 255.0 * 255.0, 255.0 * 255.0 * 255.0);
    const vec4 bit_mask = vec4(vec3(1.0 / 255.0), 0.0);
    vec4 v = fract(x * bit_shift);
    return v - v.gbaa * bit_mask;  // cut off values that do not fit in 8 bits
}


float unpack(vec4 v) {
    const vec4 bit_shift = vec4(1.0, 1.0 / 255.0, 1.0 / (255.0 * 255.0), 1.0 / (255.0 * 255.0 * 255.0));
    return dot(v, bit_shift);
}

float random1D(float x) {
    return fract(sin(x) * 100000.0);
}

float random2D(const vec2 uv) {
    const float a = 12.9898, b = 78.233, c = 43758.5453123;
    float dt = dot(uv.xy, vec2(a, b));
    float sn = mod(dt, PI);
    return fract(sin(sn) * c);
}

float QuadraticEaseIn(float x) {
    return x * x;
}

float QuadraticEaseOut(float x) {
    return x * (2 - x);
}

float QuadraticEaseInOut(float x) {
    return x < 0.5 ? (2 * x * x) : (4 * x - 2 * x * x - 1);
}

float CubicEaseIn(float x) {
    return x * x * x;
}

float CubicEaseOut(float x) {
    return 1 + pow3(x - 1);
}

float CubicEaseInOut(float x) {
    return (x < 0.5) ? (4 * x * x * x) : (0.5 * pow3(2 * x - 2) + 1);
}

float QuarticEaseIn(float x) {
    return x * x * x * x;
}

float QuarticEaseOut(float x) {
    return pow3(x - 1) * (1 - x) + 1;
}

float QuarticEaseInOut(float x) {
    return (x < 0.5) ? (8 * x * x * x * x) : (1 - 8 * pow4(x - 1));
}

float QuinticEaseIn(float x) {
    return x * x * x * x * x;
}

float QuinticEaseOut(float x) {
    return pow5(x - 1) + 1;
}

float QuinticEaseInOut(float x) {
    return (x < 0.5) ? (16 * x * x * x * x * x) : (0.5 * pow5((2 * x) - 2) + 1);
}

float SineEaseIn(float x) {
    return 1 - cos(x * HLF_PI);
}

float SineEaseOut(float x) {
    return sin(x * HLF_PI);
}

float SineEaseInOut(float x) {
    return 0.5 * (1 - cos(PI * x));
}

float CircularEaseIn(float x) {
    return 1 - sqrt(1 - x * x);
}

float CircularEaseOut(float x) {
    return sqrt((2 - x) * x);
}

float CircularEaseInOut(float x) {
    return (x < 0.5) ? (0.5 * (1 - sqrt(1 - 4 * x * x))) : (0.5 * (sqrt((3 - 2 * x) * (2 * x - 1)) + 1));
}

float ExponentialEaseIn(float x) {
    return (x == 0.0) ? x : pow(2, 10 * (x - 1));
}

float ExponentialEaseOut(float x) {
    return (x == 1.0) ? x : 1 - pow(2, -10 * x);
}

float ExponentialEaseInOut(float x) {
    return (x == 0.0 || x == 1.0) ? x :
        ((x < 0.5) ? (0.5 * pow(2, (20 * x) - 10)) : (1 - 0.5 * pow(2, (-20 * x) + 10)))
    ;
}

float ElasticEaseIn(float x) {
    return sin(13 * HLF_PI * x) * pow(2, 10 * (x - 1));
}

float ElasticEaseOut(float x) {
    return 1 - sin(13 * HLF_PI * (x + 1)) * pow(2, -10 * x);
}

float ElasticEaseInOut(float x) {
    return (x < 0.5)
        ? (0.5 * sin(26 * HLF_PI * x) * pow(2, 10 * (2 * x - 1)))
        : (0.5 * (2 - sin(26 * HLF_PI * x) * pow(2, -10 * (2 * x - 1))))
    ;
}

float BackEaseIn(float x) {
    return x * x * x - x * sin(x * PI);
}

float BackEaseOut(float x) {
    float f = 1 - x;
    return 1 - (f * f * f - f * sin(f * PI));
}

float BackEaseInOut(float x) {
    if (x < 0.5) {
        float f = 2 * x;
        return 0.5 * (f * f * f - f * sin(f * PI));
    }

    float f = 2 - 2 * x;
    return 0.5 * (1 - (f * f * f - f * sin(f * PI))) + 0.5;
}

float BounceEaseOut(float x) {
    if (x < 0.36363636363) { return x * x * 7.5625; }
    if (x < 0.72727272727) { return x * x * 9.075 - x * 9.9 + 3.4; }
    if (x < 0.90000000000) { return x * x * 12.0664819945 - x * 19.6354570637 + 8.89806094183; }
    return x * x * 10.8 - x * 20.52 + 10.72;
}

float BounceEaseIn(float x) {
    return 1 - BounceEaseOut(1 - x);
}

float BounceEaseInOut(float x) {
    return (x < 0.5) ? (0.5 * BounceEaseIn(x * 2)) : (0.5 * BounceEaseOut(x * 2 - 1) + 0.5);
}

float RadicalInverse_VdC(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;  // (1 / 0x100000000)
}

vec2 Hammersley2D(uint i, uint N) {
    return vec2(float(i) / float(N), RadicalInverse_VdC(i));
}

vec3 UniformSampleSphere(float u, float v) {
    float phi = v * PI2;
    float cos_theta = 1.0 - 2.0 * u;  // ~ [-1, 1]
    float sin_theta = sqrt(max(0, 1 - cos_theta * cos_theta));
    return vec3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);
}

vec3 UniformSampleHemisphere(float u, float v) {
    float phi = v * PI2;
    float cos_theta = 1.0 - u;  // ~ [0, 1]
    float sin_theta = sqrt(1.0 - cos_theta * cos_theta);
    return vec3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);
}

vec3 CosineSampleHemisphere(float u, float v) {
    float phi = v * PI2;
    float cos_theta = sqrt(1.0 - u);  // bias toward cosine using the `sqrt` function
    float sin_theta = sqrt(1.0 - cos_theta * cos_theta);
    return vec3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);
}

vec3 ImportanceSampleGGX(float u, float v, float alpha) {
    float a2 = alpha * alpha;
    float phi = u * PI2;
    float cos_theta = sqrt((1.0 - v) / (1.0 + (a2 - 1.0) * v));  // bias toward cosine and TRGGX NDF
    float sin_theta = sqrt(1.0 - cos_theta * cos_theta);
    return vec3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);
}

// uniformly sample a 2D point on a planar circular disk
vec2 UniformSampleDisk(float u, float v) {
    float radius = sqrt(u);
    float theta = v * PI2;
    return vec2(radius * cos(theta), radius * sin(theta));
}

// compute an array of Poisson samples on the unit disk, used for PCSS shadows
void PoissonSampleDisk(float seed, inout vec2 samples[16]) {
    const int n_samples = 16;
    float radius_step = 1.0 / float(n_samples);
    float angle_step = 3.883222077450933;  // PI2 * float(n_rings) / float(n_samples)

    float radius = radius_step;
    float angle = random1D(seed) * PI2;

    for (int i = 0; i < n_samples; ++i) {
        samples[i] = vec2(cos(angle), sin(angle)) * pow(radius, 0.75);  // 0.75 is key
        radius += radius_step;
        angle += angle_step;
    }
}

float Fd_Burley(float alpha, float NoV, float NoL, float HoL) {
    float F90 = 0.5 + 2.0 * HoL * HoL * alpha;
    float a = 1.0 + (F90 - 1.0) * pow5(1.0 - NoL);
    float b = 1.0 + (F90 - 1.0) * pow5(1.0 - NoV);
    return a * b * INV_PI;
}

float Fd_Lambert() {
    return INV_PI;
}

float Fd_Wrap(float NoL, float w) {
    float x = pow2(1.0 + w);
    return clamp((NoL + w) / x, 0.0, 1.0);
}

float D_TRGGX(float alpha, float NoH) {
    float a = NoH * alpha;
    float k = alpha / (1.0 - NoH * NoH + a * a);
    return k * k * INV_PI;
}

float D_GTR1(float alpha, float NoH) {
    if (alpha >= 1.0) return INV_PI;  // singularity case when gamma = alpha = 1
    float a2 = alpha * alpha;
    float t = 1.0 + (a2 - 1.0) * NoH * NoH;
    return (a2 - 1.0) / (PI * log(a2) * t);
}

float D_AnisoGTR2(float at, float ab, float ToH, float BoH, float NoH) {
    float a2 = at * ab;
    vec3 d = vec3(ab * ToH, at * BoH, a2 * NoH);
    float d2 = dot(d, d);
    float b2 = a2 / d2;
    return a2 * b2 * b2 * INV_PI;
}

float D_Ashikhmin(float alpha, float NoH) {
    float a2 = alpha * alpha;
    float cos2 = NoH * NoH;
    float sin2 = 1.0 - cos2;
    float sin4 = sin2 * sin2;
    float cot2 = -cos2 / (a2 * sin2);
    return 1.0 / (PI * (4.0 * a2 + 1.0) * sin4) * (4.0 * exp(cot2) + sin4);
}

float D_Charlie(float alpha, float NoH) {
    float inv_alpha = 1.0 / alpha;
    float cos2 = NoH * NoH;
    float sin2 = 1.0 - cos2;
    return (2.0 + inv_alpha) * pow(sin2, inv_alpha * 0.5) / PI2;
}

float G_SmithGGX_IBL(float NoV, float NoL, float alpha) {
    float k = alpha / 2.0;
    float GGXV = NoV / (NoV * (1.0 - k) + k);  // Schlick-GGX from view direction V
    float GGXL = NoL / (NoL * (1.0 - k) + k);  // Schlick-GGX from light direction L
    return GGXV * GGXL;
}

float G_SmithGGX(float NoV, float NoL, float roughness) {
    float r = (roughness + 1.0) * 0.5;  // remap roughness before squaring to reduce hotness
    return G_SmithGGX_IBL(NoV, NoL, r * r);
}

float V_SmithGGX(float alpha, float NoV, float NoL) {
    float a2 = alpha * alpha;
    float GGXV = NoL * sqrt(NoV * NoV * (1.0 - a2) + a2);
    float GGXL = NoV * sqrt(NoL * NoL * (1.0 - a2) + a2);
    return 0.5 / (GGXV + GGXL);
}

float V_AnisoSmithGGX(float at, float ab, float ToV, float BoV, float ToL, float BoL, float NoV, float NoL) {
    float GGXV = NoL * length(vec3(at * ToV, ab * BoV, NoV));
    float GGXL = NoV * length(vec3(at * ToL, ab * BoL, NoL));
    return 0.5 / (GGXV + GGXL);
}

float V_Kelemen(float HoL) {
    return 0.25 / (HoL * HoL);
}

float V_Neubelt(float NoV, float NoL) {
    return 1.0 / (4.0 * (NoL + NoV - NoL * NoV));
}

vec3 F_Schlick(const vec3 F0, float F90, float HoV) {
    return F0 + (F90 - F0) * pow5(1.0 - HoV);  // HoV = HoL
}

// Schlick's approximation when grazing angle reflectance F90 is assumed to be 1
vec3 F_Schlick(const vec3 F0, float HoV) {
    return F0 + (1.0 - F0) * pow5(1.0 - HoV);  // HoV = HoL
}

vec3 MultiBounceGTAO(float visibility, const vec3 albedo) {
    vec3 a =  2.0404 * albedo - 0.3324;
    vec3 b = -4.7951 * albedo + 0.6417;
    vec3 c =  2.7552 * albedo + 0.6903;
    float v = visibility;
    return max(vec3(v), ((v * a + b) * v + c) * v);
}

vec3 Reinhard(vec3 radiance) {
    return radiance / (1.0 + radiance);
}

vec3 ReinhardLuminance(vec3 radiance, float max_luminance) {
    float li = luminance(radiance);
    float numerator = li * (1.0 + (li / (max_luminance * max_luminance)));
    float lo = numerator / (1.0 + li);
    return radiance * (lo / li);
}

vec3 ReinhardJodie(vec3 radiance) {
    vec3 t = radiance / (1.0 + radiance);
    vec3 x = radiance / (1.0 + luminance(radiance));
    return vec3(mix(x.r, t.r, t.r), mix(x.g, t.g, t.g), mix(x.b, t.b, t.b));
}

vec3 Uncharted2Partial(vec3 x) {
    float a = 0.15;
    float b = 0.50;
    float c = 0.10;
    float d = 0.20;
    float e = 0.02;
    float f = 0.30;
    return ((x * (a * x + c * b) + d * e) / (x * (a * x + b) + d * f)) - e/f;
}

vec3 Uncharted2Filmic(vec3 radiance) {
    float exposure_bias = 2.0;
    vec3 white_scale = vec3(1.0) / Uncharted2Partial(vec3(11.2));
    vec3 c = Uncharted2Partial(radiance * exposure_bias);
    return c * white_scale;
}

vec3 ApproxACES(vec3 radiance) {
    vec3 v = radiance * 0.6;
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((v * (a * v + b)) / (v * (c * v + d) + e), 0.0, 1.0);
}

/* simple gamma correction, use an approximated power of 2.2 */

vec3 Gamma2Linear(vec3 color) {
    return pow(color, vec3(2.2));  // component-wise
}

vec3 Linear2Gamma(vec3 color) {
    return pow(color, vec3(1.0 / 2.2));  // component-wise
}

float Gamma2Linear(float grayscale) {
    return pow(grayscale, 2.2);
}

float Linear2Gamma(float grayscale) {
    return pow(grayscale, 1.0 / 2.2);
}

vec3 ComputeLD(const vec3 R, float roughness) {
    const float max_level = textureQueryLevels(prefilter_map) - 1.0;
    float miplevel = max_level * QuarticEaseIn(roughness);
    return textureLod(prefilter_map, R, miplevel).rgb;
}

mat3 ComputeTBN(const vec3 position, const vec3 normal, const vec2 uv) {
    vec3 dpx = dFdxFine(position);
    vec3 dpy = dFdyFine(position);
    vec2 duu = dFdxFine(uv);
    vec2 dvv = dFdyFine(uv);
    vec3 N = normalize(normal);
    vec3 T = normalize(dpx * dvv.t - dpy * duu.t);
    vec3 B = -normalize(cross(N, T));
    return mat3(T, B, N);
}

vec3 ComputeRAR(const Pixel px) {
    vec3 R = px.R;

    if (abs(px.anisotropy) > 1e-5) {
        vec3 aniso_B = px.anisotropy >= 0.0 ? px.aniso_B : px.aniso_T;
        vec3 aniso_T = cross(aniso_B, px.V);
        vec3 aniso_N = cross(aniso_T, aniso_B);  // anisotropic normal

        // the anisotropic pattern fully reveals at a roughness of 0.25
        vec3 N = mix(px.N, aniso_N, abs(px.anisotropy) * clamp(px.roughness * 4, 0.0, 1.0));
        R = reflect(-px.V, normalize(N));
    }

    return mix(R, px.N, px.alpha * px.alpha);
}

vec3 ComputeAlbedo(const vec3 albedo, float metalness) {
    return albedo * (1.0 - metalness);
}

vec3 ComputeF0(const vec3 albedo, float metalness, float specular) {
    vec3 dielectric_F0 = vec3(0.16 * specular * specular);
    return mix(dielectric_F0, albedo, metalness);
}

vec3 ComputeF0(float ni, float nt) {
    float x = (nt - ni) / (nt + ni);  // x = (eta - 1) / (eta + 1)
    return vec3(x * x);
}

float ComputeIOR(vec3 F0) {
    float x = sqrt(F0.x);
    return (1.0 + x) / (1.0 - x);
}

float ComputeDepthBias(const vec3 L, const vec3 N) {
    const float max_bias = 0.001;
    const float min_bias = 0.0001;
    return max(max_bias * (1.0 - dot(N, L)), min_bias);
}

void EvalRefraction(const Pixel px, out vec3 transmittance, out vec3 direction) {
    // spherical refraction
    if (px.volume == 0) {
        vec3 r_in = refract(-px.V, px.N, px.eta);
        float NoR = dot(-px.N, r_in);

        float m_thickness = px.thickness * px.NoV;  // local thickness varies
        float r_distance = m_thickness * NoR;

        vec3 T = clamp(exp(-px.absorption * r_distance), 0.0, 1.0);  // Beer–Lambert's law
        vec3 n_out = -normalize(NoR * r_in + px.N * 0.5);  // vector from the exit to sphere center
        vec3 r_out = refract(r_in, n_out, 1.0 / px.eta);

        transmittance = T;
        direction = r_out;
    }

    // cubic or flat refraction
    else if (px.volume == 1) {
        vec3 r_in = refract(-px.V, px.N, px.eta);
        float NoR = dot(-px.N, r_in);

        float m_thickness = px.thickness;  // thickness is constant across the flat surface
        float r_distance = m_thickness / max(NoR, 0.001);  // refracted distance is longer

        vec3 T = clamp(exp(-px.absorption * r_distance), 0.0, 1.0);  // Beer–Lambert's law
        vec3 r_out = normalize(r_in * r_distance - px.V * 10.0);  // a fixed offset of 10.0

        transmittance = T;
        direction = r_out;
    }
}

vec3 EvalDiffuseLobe(const Pixel px, float NoV, float NoL, float HoL) {
    return px.diffuse_color * Fd_Burley(px.alpha, NoV, NoL, HoL);
}

vec3 EvalSpecularLobe(const Pixel px, const vec3 L, const vec3 H, float NoV, float NoL, float NoH, float HoL) {
    float D = 0.0;
    float V = 0.0;
    vec3  F = vec3(0.0);

    if (model_x == 3) {  // cloth specular BRDF
        D = D_Charlie(px.alpha, NoH);
        V = V_Neubelt(NoV, NoL);
        F = px.F0;  // replace Fresnel with sheen color to simulate the soft luster
    }
    else if (abs(px.anisotropy) <= 1e-5) {  // non-cloth, isotropic specular BRDF
        D = D_TRGGX(px.alpha, NoH);
        V = V_SmithGGX(px.alpha, NoV, NoL);
        F = F_Schlick(px.F0, HoL);
    }
    else {  // non-cloth, anisotropic specular BRDF
        float HoT = dot(px.aniso_T, H);
        float HoB = dot(px.aniso_B, H);
        float LoT = dot(px.aniso_T, L);
        float LoB = dot(px.aniso_B, L);
        float VoT = dot(px.aniso_T, px.V);
        float VoB = dot(px.aniso_B, px.V);
        float at = max(px.alpha * (1.0 + px.anisotropy), 0.002025);  // clamp to 0.045 ^ 2 = 0.002025
        float ab = max(px.alpha * (1.0 - px.anisotropy), 0.002025);
        D = D_AnisoGTR2(at, ab, HoT, HoB, NoH);
        V = V_AnisoSmithGGX(at, ab, VoT, VoB, LoT, LoB, NoV, NoL);
        F = F_Schlick(px.F0, HoL);
    }

    return (D * V) * F;
}

// evaluates the specular BRDF lobe of the additive clearcoat layer
vec3 EvalClearcoatLobe(const Pixel px, float NoH, float HoL, out float Fcc) {
    float D = D_TRGGX(px.clearcoat_alpha, NoH);
    float V = V_Kelemen(HoL);
    vec3  F = F_Schlick(vec3(0.04), HoL) * px.clearcoat;  // assume a fixed IOR of 1.5 (4% F0)
    Fcc = F.x;
    return (D * V) * F;
}

// evaluates the contribution of a white analytical light source of unit intensity
vec3 EvaluateAL(const Pixel px, const vec3 L) {
    float NoL = dot(px.N, L);
    if (NoL <= 0.0) return vec3(0.0);

    vec3 H = normalize(px.V + L);
    vec3 Fr = vec3(0.0);
    vec3 Fd = vec3(0.0);
    vec3 Lo = vec3(0.0);

    float NoV = px.NoV;
    float NoH = max(dot(px.N, H), 0.0);
    float HoL = max(dot(H, L), 0.0);

    if (model_x == 1) {  // standard model
        Fr = EvalSpecularLobe(px, L, H, NoV, NoL, NoH, HoL) * px.Ec;  // compensate energy
        Fd = EvalDiffuseLobe(px, NoV, NoL, HoL);
        Lo = (Fd + Fr) * NoL;
    }
    else if (model_x == 2) {  // refraction model
        Fr = EvalSpecularLobe(px, L, H, NoV, NoL, NoH, HoL) * px.Ec;  // compensate energy
        Fd = EvalDiffuseLobe(px, NoV, NoL, HoL) * (1.0 - px.transmission);
        Lo = (Fd + Fr) * NoL;
    }
    else if (model_x == 3) {  // cloth model
        Fr = EvalSpecularLobe(px, L, H, NoV, NoL, NoH, HoL);  // cloth specular needs no compensation
        Fd = EvalDiffuseLobe(px, NoV, NoL, HoL) * clamp01(px.subsurface_color + NoL);  // hack subsurface color
        float cloth_NoL = Fd_Wrap(NoL, 0.5);  // simulate subsurface scattering
        Lo = Fd * cloth_NoL + Fr * NoL;
    }

    if (model_y == 1) {  // additive clearcoat layer
        float NoLcc = max(dot(px.GN, L), 0.0);  // use geometric normal
        float NoHcc = max(dot(px.GN, H), 0.0);  // use geometric normal
        float Fcc = 0.0;
        // clearcoat only has a specular lobe, diffuse is hacked by overwriting the base roughness
        vec3 Fr_cc = EvalClearcoatLobe(px, NoHcc, HoL, Fcc);
        Lo *= (1.0 - Fcc);
        Lo += (Fr_cc * NoLcc);
    }

    return Lo;
}

/*********************************** MAIN API ***********************************/

void InitPixel(inout Pixel px, const vec3 camera_pos) {
    /*
    Pixel px;
    px._position = _position;
    px._normal   = _normal;
    px._uv       = _uv;
    px._uv2      = _uv2;
    px._tangent  = _tangent;
    px._binormal = _binormal;
    px._has_tbn  = true;
    px._has_uv2  = false;
    */

    px.position = px._position;
    px.uv = px._uv * uv_scale;
    px.TBN = px._has_tbn ? mat3(px._tangent, px._binormal, px._normal) :
        ComputeTBN(px._position, px._normal, px.uv);  // approximate using partial derivatives

    px.V = normalize(camera_pos - px.position);
    px.N = sample_normal ? normalize(px.TBN * (texture(normal_map, px.uv).rgb * 2.0 - 1.0)) : px._normal;
    px.R = reflect(-px.V, px.N);
    px.NoV = max(dot(px.N, px.V), 1e-4);

    px.GN = px._normal;  // geometric normal vector, unaffected by normal map
    px.GR = reflect(-px.V, px._normal);  // geometric reflection vector

    px.albedo = sample_albedo ? vec4(Gamma2Linear(texture(albedo_map, px.uv).rgb), 1.0) : albedo;
    px.albedo.a = sample_opacity ? texture(opacity_map, px.uv).r : px.albedo.a;
    // px.albedo.rgb *= px.albedo.a;  // pre-multiply alpha channel

    if (px.albedo.a < alpha_mask) {
        discard;
    }

    px.roughness = sample_roughness ? texture(roughness_map, px.uv).r : roughness;
    px.roughness = clamp(px.roughness, 0.045, 1.0);
    px.alpha = pow2(px.roughness);

    px.ao = sample_ao ? texture(ao_map, px.uv).rrr : vec3(ao);
    px.emission = sample_Emission ? vec4(Gamma2Linear(texture(Emission_map, px.uv).rgb), 1.0) : Emission;
    px.DFG = texture(BRDF_LUT, vec2(px.NoV, px.roughness)).rgb;

    // standard model, insulators or metals, with optional anisotropy
    if (model_x == 1) {
        px.metalness = sample_metallic ? texture(metallic_map, px.uv).r : metalness;
        px.specular = clamp(specular, 0.35, 1.0);
        px.diffuse_color = ComputeAlbedo(px.albedo.rgb, px.metalness);
        px.F0 = ComputeF0(px.albedo.rgb, px.metalness, px.specular);
        px.anisotropy = anisotropy;
        px.aniso_T = sample_anisotan ? texture(anisotan_map, px.uv).rgb : aniso_dir;
        px.aniso_T = normalize(px.TBN * px.aniso_T);
        px.aniso_B = normalize(cross(px._normal, px.aniso_T));  // use geometric normal instead of normal map
        px.Ec = 1.0 + px.F0 * (1.0 / px.DFG.y - 1.0);  // energy compensation factor >= 1.0
    }

    // refraction model, for isotropic dielectrics only
    else if (model_x == 2) {
        px.anisotropy = 0.0;
        px.metalness = 0.0;
        px.diffuse_color = px.albedo.rgb;
        px.F0 = ComputeF0(1.0, clamp(ior, 1.0, 2.33));  // no real-world ior is > 2.33 (diamonds)
        px.Ec = 1.0 + px.F0 * (1.0 / px.DFG.y - 1.0);
        px.eta = 1.0 / ior;  // air -> dielectric
        px.transmission = clamp01(transmission);
        px.absorption = -log(clamp(transmittance, 1e-5, 1.0)) / max(1e-5, tr_distance);
        px.thickness = max(thickness, 1e-5);  // max thickness of the volume, not per pixel
        px.volume = clamp(volume_type, uint(0), uint(1));
    }

    // cloth model, single-layer isotropic dielectrics w/o refraction
    else if (model_x == 3) {
        px.anisotropy = 0.0;
        px.metalness = 0.0;

        if (!sample_roughness) {
            px.roughness = px.roughness * 0.2 + 0.8;  // cloth roughness under 0.8 is unrealistic
            px.alpha = pow2(px.roughness);
        }

        px.diffuse_color = px.albedo.rgb;  // use base color as diffuse color
        px.F0 = sheen_color;  // use sheen color as specular F0
        px.subsurface_color = subsurface_color;
        px.Ec = vec3(1.0);  // subsurface scattering loses energy so needs no compensation
    }

    // additive clear coat layer
    if (model_y == 1) {
        px.clearcoat = clearcoat;
        px.clearcoat_roughness = clamp(clearcoat_roughness, 0.045, 1.0);
        px.clearcoat_alpha = pow2(px.clearcoat_roughness);

        // if the coat layer is rougher, it should overwrite the base roughness
        float max_roughness = max(px.roughness, px.clearcoat_roughness);
        float mix_roughness = mix(px.roughness, max_roughness, px.clearcoat);
        px.roughness = clamp(mix_roughness, 0.045, 1.0);
        px.alpha = pow2(px.roughness);
    }
}

// evaluates the contribution of environment IBL at the pixel
vec3 EvaluateIBL(const Pixel px) {
    vec3 Fr = vec3(0.0);  // specular reflection (the Fresnel effect), weighted by E
    vec3 Fd = vec3(0.0);  // diffuse reflection, weighted by (1 - E) * (1 - transmission)
    vec3 Ft = vec3(0.0);  // diffuse refraction, weighted by (1 - E) * transmission

    vec3 E = vec3(0.0);  // specular BRDF's total energy contribution (integral after the LD term)
    vec3 AO = px.ao;     // diffuse ambient occlusion

    if (model_x == 3) {  // cloth model
        E = px.F0 * px.DFG.z;
        AO *= MultiBounceGTAO(AO.r, px.diffuse_color);
        AO *= Fd_Wrap(px.NoV, 0.5);  // simulate subsurface scattering with a wrap diffuse term
        AO *= clamp01(px.subsurface_color + px.NoV);  // simulate subsurface color (cheap hack)
    }
    else {
        E = mix(px.DFG.xxx, px.DFG.yyy, px.F0);
        AO *= MultiBounceGTAO(AO.r, px.diffuse_color);
    }

    Fr = ComputeLD(ComputeRAR(px), px.roughness) * E;
    Fr *= px.Ec;  // apply multi-scattering energy compensation (Kulla-Conty 17 and Lagarde 18)
    Fd = texture(irradiance_map, px.N).rgb * px.diffuse_color * (1.0 - E);
    Fd *= AO;  // apply ambient occlussion and multi-scattering colored GTAO

    if (model_x == 2) {  // refraction model
        vec3 transmittance;
        vec3 r_out;
        EvalRefraction(px, transmittance, r_out);

        Ft = ComputeLD(r_out, px.roughness) * px.diffuse_color;
        Ft *= transmittance;  // apply absorption (transmittance color may differ from the base albedo)
        Fd *= (1.0 - px.transmission);  // already multiplied by (1.0 - E)
        Ft *= (1.0 - E) * px.transmission;
    }

    if (model_y == 1) {  // additive clear coat layer
        float Fcc = F_Schlick(vec3(0.04), px.NoV).x * px.clearcoat;  // polyurethane F0 = 4%
        Fd *= (1.0 - Fcc);
        Fr *= (1.0 - Fcc);
        Fr += ComputeLD(px.GR, px.clearcoat_roughness) * Fcc;
    }

    return Fr + Fd + Ft;
}

// evaluates the contribution of a white directional light of unit intensity
vec3 EvaluateADL(const Pixel px, const vec3 L, float visibility) {
    return visibility <= 0.0 ? vec3(0.0) : (EvaluateAL(px, L) * visibility);
}

// evaluates the contribution of a white point light of unit intensity
vec3 EvaluateAPL(const Pixel px, const vec3 position, float range, float linear, float quadratic, float visibility) {
    vec3 L = normalize(position - px.position);

    // distance attenuation: inverse square falloff
    float d = distance(position, px.position);
    float attenuation = (d >= range) ? 0.0 : (1.0 / (1.0 + linear * d + quadratic * d * d));

    return (attenuation <= 0.0 || visibility <= 0.0) ? vec3(0.0) : (attenuation * visibility * EvaluateAL(px, L));
}

// evaluates the contribution of a white spotlight of unit intensity
vec3 EvaluateASL(const Pixel px, const vec3 pos, const vec3 dir, float range, float inner_cos, float outer_cos) {
    vec3 l = pos - px.position;
    vec3 L = normalize(l);

    // distance attenuation uses a cheap linear falloff (does not follow the inverse square law)
    float ds = dot(dir, l);  // projected distance along the spotlight beam direction
    float da = 1.0 - clamp01(ds / range);

    // angular attenuation fades out from the inner to the outer cone
    float cosine = dot(dir, L);
    float aa = clamp01((cosine - outer_cos) / (inner_cos - outer_cos));
    float attenuation = da * aa;

    return attenuation <= 0.0 ? vec3(0.0) : (EvaluateAL(px, L) * attenuation);
}


float EvalOcclusion(const Pixel px, in samplerCube shadow_map, const vec3 light_pos, float light_radius) {
    const float near_clip = 0.1;
    const float far_clip = 100.0;
    vec3 l = light_pos - px.position;
    vec3 L = normalize(l);
    float depth = length(l) / far_clip;  
    vec3 U = mix(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), step(abs(L.y), 0.999));
    vec3 T = normalize(cross(U, -L));
    vec3 B = normalize(cross(-L, T));
    const int n_samples = 16;
    const vec2 samples[16] = vec2[] (  // source: Nvidia 2008, "PCSS Integration whitepaper"
        vec2(-0.94201624, -0.39906216), vec2( 0.94558609, -0.76890725),
        vec2(-0.09418410, -0.92938870), vec2( 0.34495938,  0.29387760),
        vec2(-0.91588581,  0.45771432), vec2(-0.81544232, -0.87912464),
        vec2(-0.38277543,  0.27676845), vec2( 0.97484398,  0.75648379),
        vec2( 0.44323325, -0.97511554), vec2( 0.53742981, -0.47373420),
        vec2(-0.26496911, -0.41893023), vec2( 0.79197514,  0.19090188),
        vec2(-0.24188840,  0.99706507), vec2(-0.81409955,  0.91437590),
        vec2( 0.19984126,  0.78641367), vec2( 0.14383161, -0.14100790)
    );

    // sample the unit disk, count blockers and total depth
    float search_radius = light_radius * depth;  // if pixel is far, search in a larger area
    float total_depth = 0.0;
    int n_blocker = 0;

    for (int i = 0; i < n_samples; ++i) {
        vec2 offset = samples[i];
        vec3 v = -L + (offset.x * T + offset.y * B) * search_radius;
        float sm_depth = texture(shadow_map, v).r;

        if (depth > sm_depth) {  // in this step we don't need a bias
            total_depth += sm_depth;
            n_blocker++;
        }
    }

    // early out if no blockers are found (100% visible)
    if (n_blocker == 0) {
        return 0.0;
    }

    // compute the average blocker depth and penumbra size
    float z_blocker = total_depth / float(n_blocker);
    float penumbra = (depth - z_blocker) / z_blocker;  // ~ [0.0, 1.0]

    // compute occlusion with PCF, this time use penumbra to determine the kernel size
    float PCF_radius = light_radius * penumbra;
    float bias = ComputeDepthBias(L, px.GN);
    float occlusion = 0.0;

    for (int i = 0; i < n_samples; ++i) {
        vec2 offset = samples[i];
        vec3 v = -L + (offset.x * T + offset.y * B) * PCF_radius;
        float sm_depth = texture(shadow_map, v).r;

        if (depth - bias > sm_depth) {
            occlusion += 1.0;
        }
    }

    return occlusion / float(n_samples);
}
vec3 UnPack(float p_Target)
{
    return vec3
    (
        float((uint(p_Target) >> 24) & 0xff)    * 0.003921568627451,
        float((uint(p_Target) >> 16) & 0xff)    * 0.003921568627451,
        float((uint(p_Target) >> 8) & 0xff)     * 0.003921568627451
    );
}
void main() {
    Pixel px;
    px._position = _position;
    px._normal   = _normal;
    px._uv       = _uv;
    px._uv2      = _uv2;
    px._tangent  = _tangent;
    px._binormal = _binormal;
    px._has_tbn  = false;
    px._has_uv2  = false;

    InitPixel(px, ubo_ViewPos);

    vec3 Lo = vec3(0.0);
    //vec3 Lo=albedo.rgb;
    vec3 Le = vec3(0.0);  // emission

    Lo += EvaluateIBL(px) * min(max(ibl_exposure, 0.5),1.0);

    for(int i=0;i<ssbo_Lights.length();i++){
        //DIRECTIONAL
        if(ssbo_Lights[i][3][0]==1.0){
         Lo += EvaluateADL(px, -ssbo_Lights[i][1].rgb, 1.0) * UnPack(ssbo_Lights[i][2][0]) * ssbo_Lights[i][3][3];
        }
        //POINT
        if(ssbo_Lights[i][3][0]==0.0){
        float visibility = 1.0;
        visibility -= EvalOcclusion(px, shadow_map1[uint(ssbo_Lights[i][2][2])], ssbo_Lights[i][0].rgb, 0.001);
        vec3 pc = EvaluateAPL(px, ssbo_Lights[i][0].rgb, ssbo_Lights[i][2][1],ssbo_Lights[i][1][3], ssbo_Lights[i][2][3], visibility);
        Lo += pc * UnPack(ssbo_Lights[i][2][0]) * ssbo_Lights[i][3][3];
        }
        //SPOT
        if(ssbo_Lights[i][3][0]==2.0){

        vec3 sc = EvaluateASL(px,ssbo_Lights[i][0].rgb,-normalize(ssbo_Lights[i][1].rgb),
        ssbo_Lights[i][2][1], cos(radians(ssbo_Lights[i][3][1])),
        cos(radians(ssbo_Lights[i][3][1] + ssbo_Lights[i][3][2])));
        Lo += 3.5*sc * UnPack(ssbo_Lights[i][2][0]) * ssbo_Lights[i][3][3];
        }

    }
    bloom = vec4(0.0,0.0,0.0,1.0);
    //color = vec4(Lo + Le, 1.0);
    color = vec4((_normal+1.0)/2.0, 1.0);
    //color = vec4(0.0,1.0,1.0, 1.0);
 
}