
#version 460 core
#pragma optimize(off)


layout(location = 1000) uniform mat4 selfmodel;
layout(location = 1001) uniform mat4 view;
layout(location = 1002) uniform mat4 projection;
layout(location = 1003) uniform vec3 cameraviewpos;

////////////////////////////////////////////////////////////////////////////////

#ifdef vertex_shader

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;

layout(location = 0) out _vtx {
    out vec3 _position;
    out vec3 _normal;
    out vec2 _uv;
};


void main() {

   

    _position = vec3(selfmodel  * vec4(position.xyz, 1.0));
    _normal   = normalize(vec3(selfmodel  * vec4(normal.xyz, 0.0)));
    _uv = vec2(position.w,normal.w); 
    gl_Position = projection*view*selfmodel* vec4(position.xyz, 1.0);
}

#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef fragment_shader


// sampler binding points (texture units) 17-19 are reserved for PBR IBL
layout(binding = 17) uniform samplerCube irradiance_map;
layout(binding = 18) uniform samplerCube prefilter_map;
layout(binding = 19) uniform sampler2D BRDF_LUT;

// sampler binding points (texture units) >= 20 are reserved for PBR use
layout(binding = 20) uniform sampler2D albedo_map;
layout(binding = 21) uniform sampler2D normal_map;
layout(binding = 22) uniform sampler2D metallic_map;
layout(binding = 23) uniform sampler2D roughness_map;
layout(binding = 24) uniform sampler2D ao_map;
layout(binding = 25) uniform sampler2D emission_map;
layout(binding = 26) uniform sampler2D displace_map;
layout(binding = 27) uniform sampler2D opacity_map;
layout(binding = 28) uniform sampler2D light_map;
layout(binding = 29) uniform sampler2D anisotan_map;  // anisotropic tangent map (RGB)
layout(binding = 30) uniform sampler2D ext_unit_30;
layout(binding = 31) uniform sampler2D ext_unit_31;

// default-block (loose) uniform locations >= 900 are reserved for PBR use
layout(location = 900) uniform bool sample_albedo;
layout(location = 901) uniform bool sample_normal;
layout(location = 902) uniform bool sample_metallic;
layout(location = 903) uniform bool sample_roughness;
layout(location = 904) uniform bool sample_ao;
layout(location = 905) uniform bool sample_emission;
layout(location = 906) uniform bool sample_displace;
layout(location = 907) uniform bool sample_opacity;
layout(location = 908) uniform bool sample_lightmap;
layout(location = 909) uniform bool sample_anisotan;
layout(location = 910) uniform bool sample_ext_910;
layout(location = 911) uniform bool sample_ext_911;

/***** physically-based material input properties *****/

// shared properties
layout(location = 912) uniform vec3  albedo;         // alpha is not pre-multiplied
layout(location = 913) uniform float roughness;      // clamped at 0.045 so that specular highlight is visible
layout(location = 914) uniform float ao;             // 0.0 = occluded, 1.0 = not occluded
layout(location = 915) uniform vec3  emission;       // optional emissive color
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

// the subsurface scattering model is very different and hard so we'll skip it for now, the way
// that Disney BSDF (2015) and Filament handles real-time SSS is very hacky, which is not worth
// learning at the moment. A decent SSS model usually involves an approximation of path tracing
// volumetrics, which I'll come back to in a future project when taking the course CMU 15-468.

// a two-digit number indicative of how the pixel should be shaded
// component x encodes the shading model: standard = 1, refraction = 2, cloth = 3
// component y encodes an additive layer: none = 0, clearcoat = 1, sheen = 2
layout(location = 999) uniform uvec2 model;

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

// returns a rgb color from the continuous rainbow bar, based on the hue param (0 ~ 1)
// to create a smooth color transition, hue must be unidirectional, so once hue hits 1
// it must jump back to 0, such a value can be created from a modulo or fract function
vec3 rainbow(float hue) {
    return hsv2rgb(hue, 1.0, 1.0);
}

// returns a float that bounces between 0.0 and k as the value of x changes monotonically
float bounce(float x, float k) {
    return k - abs(k - mod(x, k * 2));
}

// packs a 32-bit float into a vec4 (RGBA format), each component of RGBA = 8 bits and 1/256 precision
// this is often used on mobiles that do not support a high precision format like GL_DEPTH_COMPONENT32
// for example, implementations of shadow mapping in WebGL have been using this code extensively
vec4 pack(float x) {
    const vec4 bit_shift = vec4(1.0, 255.0, 255.0 * 255.0, 255.0 * 255.0 * 255.0);
    const vec4 bit_mask = vec4(vec3(1.0 / 255.0), 0.0);
    vec4 v = fract(x * bit_shift);
    return v - v.gbaa * bit_mask;  // cut off values that do not fit in 8 bits
}

// unpacks a vec4 RGBA into a 32-bit precision floating point scalar, for the explanation see:
// https://stackoverflow.com/questions/9882716/packing-float-into-vec4-how-does-this-code-work
float unpack(vec4 v) {
    const vec4 bit_shift = vec4(1.0, 1.0 / 255.0, 1.0 / (255.0 * 255.0), 1.0 / (255.0 * 255.0 * 255.0));
    return dot(v, bit_shift);
}

// returns a naive pseudo-random number between 0 and 1, seed x can be any number within (-inf, +inf)
// note that this randomness is more concentrated around 0.5 and sparser at the two ends 0.0 and 1.0
// it's extremely flawed at the sinusoid's peaks, so seed x should not be close to multiples of PI/2
// source: https://thebookofshaders.com/10/
float random1D(float x) {
    return fract(sin(x) * 100000.0);
}

// returns a naive pseudo-random number between 0 and 1, based on a 2D seed vector
// the random pattern can be changed using different values of a, b and c, but the
// seed value (for each component) must be a floating point number between 0 and 1
float random2D(const vec2 uv) {
    const float a = 12.9898, b = 78.233, c = 43758.5453123;
    float dt = dot(uv.xy, vec2(a, b));
    float sn = mod(dt, PI);
    return fract(sin(sn) * c);
}

// some cheap easing functions (branchless) adapted from https://github.com/warrenm/AHEasing
// the effects of these easing functions can be visualized at https://easings.net/

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
// reference:
// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
// https://www.pbr-book.org/3ed-2018/Monte_Carlo_Integration/2D_Sampling_with_Multidimensional_Transformations
// https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
// the Van der Corput radical inverse sequence
float RadicalInverse_VdC(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;  // (1 / 0x100000000)
}

// the Hammersley point set (a low-discrepancy random sequence)
vec2 Hammersley2D(uint i, uint N) {
    return vec2(float(i) / float(N), RadicalInverse_VdC(i));
}

// uniformly sample a point (also a direction vector) on the unit sphere
// the probability of a point being sampled is 1 / (4 * PI), i.e. unbiased
vec3 UniformSampleSphere(float u, float v) {
    float phi = v * PI2;
    float cos_theta = 1.0 - 2.0 * u;  // ~ [-1, 1]
    float sin_theta = sqrt(max(0, 1 - cos_theta * cos_theta));
    return vec3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);
}

// uniformly sample a point (also a direction vector) on the unit hemisphere
// the probability of a point being sampled is 1 / (2 * PI), i.e. unbiased
vec3 UniformSampleHemisphere(float u, float v) {
    float phi = v * PI2;
    float cos_theta = 1.0 - u;  // ~ [0, 1]
    float sin_theta = sqrt(1.0 - cos_theta * cos_theta);
    return vec3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);
}

// cosine-weighted point sampling on the unit hemisphere
// the probability of a point being sampled is (cosine / PI), i.e. biased by cosine
// this method is favored over uniform sampling for cosine-weighted rendering equations
vec3 CosineSampleHemisphere(float u, float v) {
    float phi = v * PI2;
    float cos_theta = sqrt(1.0 - u);  // bias toward cosine using the `sqrt` function
    float sin_theta = sqrt(1.0 - cos_theta * cos_theta);
    return vec3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);
}

// importance sampling with GGX NDF for a given alpha (roughness squared)
// this also operates on the unit hemisphere, and the PDF is D_TRGGX() * cosine
// this function returns the halfway vector H (because NDF is evaluated at H)
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
/* this is a simplified Disney BSDF material model adapted from Google Filament, for theory
   and details behind physically based rendering, check out the Filament core documentation
   and the SIGGRAPH physically based shading course series since 2012.

   reference:
   - https://google.github.io/filament/Filament.html
   - https://google.github.io/filament/Materials.html
   - https://blog.selfshadow.com/publications/
   - http://www.advances.realtimerendering.com/
   - https://pbr-book.org/3ed-2018/contents
*/
/* ------------------------------ Diffuse BRDF model ------------------------------ */

// Disney's diffuse BRDF that takes account of roughness (not energy conserving though)
// Brent Burley 2012, "Physically Based Shading at Disney"
float Fd_Burley(float alpha, float NoV, float NoL, float HoL) {
    float F90 = 0.5 + 2.0 * HoL * HoL * alpha;
    float a = 1.0 + (F90 - 1.0) * pow5(1.0 - NoL);
    float b = 1.0 + (F90 - 1.0) * pow5(1.0 - NoV);
    return a * b * INV_PI;
}

// Lambertian diffuse BRDF that assumes a uniform response over the hemisphere H2, note
// that 1/PI comes from the energy conservation constraint (integrate BRDF over H2 = 1)
float Fd_Lambert() {
    return INV_PI;
}

// Cloth diffuse BRDF approximated using a wrap diffuse term (energy conserving)
// source: Physically Based Rendering in Filament documentation, section 4.12.2
float Fd_Wrap(float NoL, float w) {
    float x = pow2(1.0 + w);
    return clamp((NoL + w) / x, 0.0, 1.0);
}

/* ------------------------- Specular D - Normal Distribution Function ------------------------- */

// Trowbridge-Reitz GGX normal distribution function (long tail distribution)
// Bruce Walter et al. 2007, "Microfacet Models for Refraction through Rough Surfaces"
float D_TRGGX(float alpha, float NoH) {
    float a = NoH * alpha;
    float k = alpha / (1.0 - NoH * NoH + a * a);
    return k * k * INV_PI;
}

// Generalized Trowbridge-Reitz NDF when gamma = 1 (tail is even longer)
// Brent Burley 2012, "Physically Based Shading at Disney"
float D_GTR1(float alpha, float NoH) {
    if (alpha >= 1.0) return INV_PI;  // singularity case when gamma = alpha = 1
    float a2 = alpha * alpha;
    float t = 1.0 + (a2 - 1.0) * NoH * NoH;
    return (a2 - 1.0) / (PI * log(a2) * t);
}

// GTR2 (equivalent to GGX) anisotropic normal distribution function
// Brent Burley 2012, "Physically Based Shading at Disney"
float D_AnisoGTR2(float at, float ab, float ToH, float BoH, float NoH) {
    float a2 = at * ab;
    vec3 d = vec3(ab * ToH, at * BoH, a2 * NoH);
    float d2 = dot(d, d);
    float b2 = a2 / d2;
    return a2 * b2 * b2 * INV_PI;
}

// Ashikhmin's inverted Gaussian based velvet distribution, normalized by Neubelt
// Ashikhmin and Premoze 2007, "Distribution-based BRDFs"
// Neubelt 2013, "Crafting a Next-Gen Material Pipeline for The Order: 1886"
float D_Ashikhmin(float alpha, float NoH) {
    float a2 = alpha * alpha;
    float cos2 = NoH * NoH;
    float sin2 = 1.0 - cos2;
    float sin4 = sin2 * sin2;
    float cot2 = -cos2 / (a2 * sin2);
    return 1.0 / (PI * (4.0 * a2 + 1.0) * sin4) * (4.0 * exp(cot2) + sin4);
}

// Charlie distribution function based on an exponentiated sinusoidal
// Estevez and Kulla 2017, "Production Friendly Microfacet Sheen BRDF"
float D_Charlie(float alpha, float NoH) {
    float inv_alpha = 1.0 / alpha;
    float cos2 = NoH * NoH;
    float sin2 = 1.0 - cos2;
    return (2.0 + inv_alpha) * pow(sin2, inv_alpha * 0.5) / PI2;
}
/* ------------------------- Specular G - Geometry Function (shadowing-masking) ------------------------- */

// Smith's geometry function (used for indirect image-based lighting)
// Schlick 1994, "An Inexpensive BRDF Model for Physically-based Rendering"
float G_SmithGGX_IBL(float NoV, float NoL, float alpha) {
    float k = alpha / 2.0;
    float GGXV = NoV / (NoV * (1.0 - k) + k);  // Schlick-GGX from view direction V
    float GGXL = NoL / (NoL * (1.0 - k) + k);  // Schlick-GGX from light direction L
    return GGXV * GGXL;
}
// Smith's geometry function modified for analytic light sources
// Burley 2012, "Physically Based Shading at Disney", Karis 2013, "Real Shading in Unreal Engine 4"
float G_SmithGGX(float NoV, float NoL, float roughness) {
    float r = (roughness + 1.0) * 0.5;  // remap roughness before squaring to reduce hotness
    return G_SmithGGX_IBL(NoV, NoL, r * r);
}

// Smith's height-correlated visibility function (V = G / normalization term)
// Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"
float V_SmithGGX(float alpha, float NoV, float NoL) {
    float a2 = alpha * alpha;
    float GGXV = NoL * sqrt(NoV * NoV * (1.0 - a2) + a2);
    float GGXL = NoV * sqrt(NoL * NoL * (1.0 - a2) + a2);
    return 0.5 / (GGXV + GGXL);
}

// height-correlated GGX distribution anisotropic visibility function
// Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"
float V_AnisoSmithGGX(float at, float ab, float ToV, float BoV, float ToL, float BoL, float NoV, float NoL) {
    float GGXV = NoL * length(vec3(at * ToV, ab * BoV, NoV));
    float GGXL = NoV * length(vec3(at * ToL, ab * BoL, NoL));
    return 0.5 / (GGXV + GGXL);
}

// Kelemen's visibility function for clear coat specular BRDF
// Kelemen 2001, "A Microfacet Based Coupled Specular-Matte BRDF Model with Importance Sampling"
float V_Kelemen(float HoL) {
    return 0.25 / (HoL * HoL);
}

// Neubelt's smooth visibility function for use with cloth and velvet distribution
// Neubelt and Pettineo 2013, "Crafting a Next-gen Material Pipeline for The Order: 1886"
float V_Neubelt(float NoV, float NoL) {
    return 1.0 / (4.0 * (NoL + NoV - NoL * NoV));
}

/* ------------------------- Specular F - Fresnel Reflectance Function ------------------------- */

// Schlick's approximation of specular reflectance (the Fresnel factor)
// Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"
vec3 F_Schlick(const vec3 F0, float F90, float HoV) {
    return F0 + (F90 - F0) * pow5(1.0 - HoV);  // HoV = HoL
}

// Schlick's approximation when grazing angle reflectance F90 is assumed to be 1
vec3 F_Schlick(const vec3 F0, float HoV) {
    return F0 + (1.0 - F0) * pow5(1.0 - HoV);  // HoV = HoL
}

/* ------------------------- Ambient Occlusion, Misc ------------------------- */

// Ground truth based colored ambient occlussion (colored GTAO)
// Jimenez et al. 2016, "Practical Realtime Strategies for Accurate Indirect Occlusion"
vec3 MultiBounceGTAO(float visibility, const vec3 albedo) {
    vec3 a =  2.0404 * albedo - 0.3324;
    vec3 b = -4.7951 * albedo + 0.6417;
    vec3 c =  2.7552 * albedo + 0.6903;
    float v = visibility;
    return max(vec3(v), ((v * a + b) * v + c) * v);
}
/* commonly used tone mapping operators (TMO) */
// https://64.github.io/tonemapping/
// https://docs.unrealengine.com/4.26/en-US/RenderingAndGraphics/PostProcessEffects/ColorGrading/

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
// this shader implements our physically based rendering API, reference:
// https://google.github.io/filament/Filament.html
// https://blog.selfshadow.com/publications/
// https://pbr-book.org/3ed-2018/contents

/* computes the LD term (from prefiltered envmap) in the split-sum equation

   we're using a non-linear mapping between roughness and mipmap level here to
   emphasize specular reflection, this quartic ease in function will map most
   surfaces with a roughness < 0.5 to the base level of the prefiltered envmap.
   depending on the context, people might prefer rough appearance over strongly
   glossy smooth surfaces, this often looks better and much more realistic. In
   this case, you can fit roughness using a cubic ease out function, so that
   most surfaces will sample from higher mip levels, and specular IBL will be
   limited to highly smooth surfaces only.
*/
vec3 ComputeLD(const vec3 R, float roughness) {
    const float max_level = textureQueryLevels(prefilter_map) - 1.0;
    float miplevel = max_level * QuarticEaseIn(roughness);
    return textureLod(prefilter_map, R, miplevel).rgb;
}

/* computes the tangent-binormal-normal (TBN) matrix at the current pixel

   the TBN matrix is used to convert a vector from tangent space to world space
   if tangents and binormals are not provided by VBO as vertex attributes input
   this function will use local partial derivatives to approximate the matrix.
   if tangents and binormals are already provided, we don't need this function
   at all unless the interpolated T, B and N vectors could lose orthogonality.
*/
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

/* computes the roughness-anisotropy-corrected reflection vector RAR

   for a perfect specular surface, we use the reflection vector R to fetch the IBL.
   for 100% diffuse, normal vector N is used instead as diffuse is view-independent.
   given a roughness ~ [0, 1], we now have a mixture of diffuse and specular, so we
   should interpolate between the two based on how rough the surface is, that gives
   the vector RAR. RAR also takes account of anisotropy, thus the letter A.

   the RAR vector is only meant to compute the specular LD term. As for the diffuse
   part, we should still use the isotropic normal vector N to fetch the precomputed
   irradiance map or evaluate SH9 regardless of anisotropy. Anisotropy only applies
   to brushed metals so it's mainly concerned with specular component.
*/
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

/* computes diffuse color from the base color (albedo) and metalness
   for dielectrics, diffuse color is the persistent base color, F0 is achromatic
   for conductors, there's no diffuse color, diffuse = vec3(0), F0 is chromatic
*/
vec3 ComputeAlbedo(const vec3 albedo, float metalness) {
    return albedo * (1.0 - metalness);
}

/* computes F0 from the base color (albedo), metalness and specular
   this is chromatic for conductors and achromatic for dielectrics
*/
vec3 ComputeF0(const vec3 albedo, float metalness, float specular) {
    vec3 dielectric_F0 = vec3(0.16 * specular * specular);
    return mix(dielectric_F0, albedo, metalness);
}

/* computes F0 from the incident IOR and transmitted IOR (index of refraction)
   this assumes an air-dielectric interface where the incident IOR of air = 1

   by convention, nt = transmitted IOR, ni = incident IOR, and the ratio of IOR
   is denoted by eta = nt / ni, but in our case the order of ni and nt does not
   matter as we are taking the expression squared where ni and nt are symmetric
*/
vec3 ComputeF0(float ni, float nt) {
    float x = (nt - ni) / (nt + ni);  // x = (eta - 1) / (eta + 1)
    return vec3(x * x);
}

/* computes IOR of the refraction medium from F0, assumes an air-dielectric interface
   the value returned is really eta = nt / ni, but since ni = 1, it's essentially IOR
*/
float ComputeIOR(vec3 F0) {
    float x = sqrt(F0.x);
    return (1.0 + x) / (1.0 - x);
}

/* computes the bias term that is used to offset the shadow map and remove shadow acne
   we need more bias when NoL is small, less when NoL is large (perpendicular surfaces)
   if the resolution of the shadow map is high, the min/max bias values can be reduced
   N and L must be normalized, and N must be geometric normal, not from the normal map
*/
float ComputeDepthBias(const vec3 L, const vec3 N) {
    const float max_bias = 0.001;
    const float min_bias = 0.0001;
    return max(max_bias * (1.0 - dot(N, L)), min_bias);
}

/* evaluates the path of refraction at the pixel, assumes an air-dielectric interface
   the exact behavior of refraction depends on the medium's interior geometric structures
   for simplicity we only consider uniform solid volumes in the form of spheres or cubes

   if light enters a medium whose volume is a uniform sphere, cylinder or capsule, it is
   spherically distorted, and each point on the surface has a different local thickness.
   local thickness drops from diameter d to 0 as we go from the sphere center to the rim

   if light enters a uniform flat volume such as a cube, plastic bar or glass plate, it's
   not distorted but shifted due to the thickness of the volume. The entry/exit interface
   are symmetric to each other, which implies that the exit direction of light equals the
   entry direction, that's the view vector V. Therefore, when sampling the infinitely far
   environment map, we won't be able to observe the varying shifts as would o/w appear in
   real life. For this, we adopt a cheap solution by adding a hardcoded offset.

   if local light probes were to be used instead of distant IBL, we would need another
   function because sampling local IBL depends on the position as well
*/
void EvalRefraction(const Pixel px, out vec3 transmittance, out vec3 direction) {
    // spherical refraction
    if (px.volume == 0) {
        vec3 r_in = refract(-px.V, px.N, px.eta);
        float NoR = dot(-px.N, r_in);

        float m_thickness = px.thickness * px.NoV;  // local thickness varies
        float r_distance = m_thickness * NoR;

        vec3 T = clamp(exp(-px.absorption * r_distance), 0.0, 1.0);  // Beer�CLambert's law
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

        vec3 T = clamp(exp(-px.absorption * r_distance), 0.0, 1.0);  // Beer�CLambert's law
        vec3 r_out = normalize(r_in * r_distance - px.V * 10.0);  // a fixed offset of 10.0

        transmittance = T;
        direction = r_out;
    }
}

// evaluates base material's diffuse BRDF lobe
vec3 EvalDiffuseLobe(const Pixel px, float NoV, float NoL, float HoL) {
    return px.diffuse_color * Fd_Burley(px.alpha, NoV, NoL, HoL);
}

// evaluates base material's specular BRDF lobe
vec3 EvalSpecularLobe(const Pixel px, const vec3 L, const vec3 H, float NoV, float NoL, float NoH, float HoL) {
    float D = 0.0;
    float V = 0.0;
    vec3  F = vec3(0.0);

    if (model.x == 3) {  // cloth specular BRDF
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

        // Brent Burley 2012, "Physically Based Shading at Disney"
        // float aspect = sqrt(1.0 - 0.9 * px.anisotropy);
        // float at = max(px.alpha / aspect, 0.002025);  // alpha along the tangent direction
        // float ab = max(px.alpha * aspect, 0.002025);  // alpha along the binormal direction

        // Kulla 2017, "Revisiting Physically Based Shading at Imageworks"
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

    if (model.x == 1) {  // standard model
        Fr = EvalSpecularLobe(px, L, H, NoV, NoL, NoH, HoL) * px.Ec;  // compensate energy
        Fd = EvalDiffuseLobe(px, NoV, NoL, HoL);
        Lo = (Fd + Fr) * NoL;
    }
    else if (model.x == 2) {  // refraction model
        Fr = EvalSpecularLobe(px, L, H, NoV, NoL, NoH, HoL) * px.Ec;  // compensate energy
        Fd = EvalDiffuseLobe(px, NoV, NoL, HoL) * (1.0 - px.transmission);
        Lo = (Fd + Fr) * NoL;
    }
    else if (model.x == 3) {  // cloth model
        Fr = EvalSpecularLobe(px, L, H, NoV, NoL, NoH, HoL);  // cloth specular needs no compensation
        Fd = EvalDiffuseLobe(px, NoV, NoL, HoL) * clamp01(px.subsurface_color + NoL);  // hack subsurface color
        float cloth_NoL = Fd_Wrap(NoL, 0.5);  // simulate subsurface scattering
        Lo = Fd * cloth_NoL + Fr * NoL;
    }

    if (model.y == 1) {  // additive clearcoat layer
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

// initializes the current pixel (fragment), values are computed from the material inputs
void InitPixel(inout Pixel px, const vec3 camera_pos) {
    px.position = px._position;
    px.uv = px._uv* uv_scale;
    px.TBN = px._has_tbn ? mat3(px._tangent, px._binormal, px._normal) :
        ComputeTBN(px._position, px._normal, px.uv);  // approximate using partial derivatives

    px.V = normalize(camera_pos - px.position);
    px.N = sample_normal ? normalize(px.TBN * (texture(normal_map, px.uv).rgb * 2.0 - 1.0)) : px._normal;
   // px.N = px._normal;
    px.R = reflect(-px.V, px.N);
    px.NoV = max(dot(px.N, px.V), 1e-4);

    px.GN = px._normal;  // geometric normal vector, unaffected by normal map
    px.GR = reflect(-px.V, px._normal);  // geometric reflection vector

    px.albedo = sample_albedo ? vec4(Gamma2Linear(texture(albedo_map, px.uv).rgb), 1.0) :vec4(albedo,1.0) ;
    px.albedo.a = sample_opacity ? texture(opacity_map, px.uv).r : px.albedo.a;

    // px.albedo.rgb *= px.albedo.a;  // pre-multiply alpha channel



    px.roughness = sample_roughness ? texture(roughness_map, px.uv).r : roughness;
    px.roughness = clamp(px.roughness, 0.045, 1.0);
    px.alpha = pow2(px.roughness);

    px.ao = sample_ao ? texture(ao_map, px.uv).rrr : vec3(ao);
    px.emission = sample_emission ? vec4(Gamma2Linear(texture(emission_map, px.uv).rgb), 1.0) : vec4(emission,1.0);
    px.DFG = texture(BRDF_LUT, vec2(px.NoV, px.roughness)).rgb;

    // standard model, insulators or metals, with optional anisotropy
    if (model.x == 1) {
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

}

// evaluates the contribution of environment IBL at the pixel
vec3 EvaluateIBL(const Pixel px) {
    vec3 Fr = vec3(0.0);  // specular reflection (the Fresnel effect), weighted by E
    vec3 Fd = vec3(0.0);  // diffuse reflection, weighted by (1 - E) * (1 - transmission)
    vec3 Ft = vec3(0.0);  // diffuse refraction, weighted by (1 - E) * transmission

    vec3 E = vec3(0.0);  // specular BRDF's total energy contribution (integral after the LD term)
    vec3 AO = px.ao;     // diffuse ambient occlusion

    if (model.x == 3) {  // cloth model
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

    // the irradiance map already includes the Lambertian BRDF so we multiply the texel by
    // diffuse color directly. Do not divide by PI here cause that will be double-counting
    // for spherical harmonics, INV_PI should be rolled into SH9 during C++ precomputation

    Fd = texture(irradiance_map, px.N).rgb * px.diffuse_color * (1.0 - E);
    Fd *= AO;  // apply ambient occlussion and multi-scattering colored GTAO

    if (model.x == 2) {  // refraction model
        vec3 transmittance;
        vec3 r_out;
        EvalRefraction(px, transmittance, r_out);

        Ft = ComputeLD(r_out, px.roughness) * px.diffuse_color;
        Ft *= transmittance;  // apply absorption (transmittance color may differ from the base albedo)

        // note that reflection and refraction are mutually exclusive, photons that bounce off
        // the surface do not enter the object, so the presence of refraction will only eat up
        // some of diffuse's contribution but will not affect the specular part

        Fd *= (1.0 - px.transmission);  // already multiplied by (1.0 - E)
        Ft *= (1.0 - E) * px.transmission;
    }

    if (model.y == 1) {  // additive clear coat layer
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

/* evaluates the amount of occlusion for a single light source using the PCSS algorithm

   this function works with omni-directional SM in linear space, the shadow map must be
   a cubemap depth texture that stores linear values, note that this is just a hack for
   casting soft shadows from a point light, spotlight or directional light, but in real
   life they really should be hard shadows since only area lights can cast soft shadows.

   for PCF, texels are picked using Poisson disk sampling which favors samples that are
   more nearby, it can preserve the shadow shape very well even when `n_samples` or the
   search radius is large, which isn't true for uniform disk sampling where shadows are
   often overly blurred. To increase shadow softness, you can use a larger light radius.

   https://sites.cs.ucsb.edu/~lingqi/teaching/resources/GAMES202_Lecture_03.pdf
   https://developer.download.nvidia.cn/shaderlibrary/docs/shadow_PCSS.pdf
   https://developer.download.nvidia.cn/whitepapers/2008/PCSS_Integration.pdf
   https://pbr-book.org/3ed-2018/Monte_Carlo_Integration/2D_Sampling_with_Multidimensional_Transformations
*/
float EvalOcclusion(const Pixel px, in samplerCube shadow_map, const vec3 light_pos, float light_radius) {
    const float near_clip = 0.1;
    const float far_clip = 100.0;
    vec3 l = light_pos - px.position;
    vec3 L = normalize(l);
    float depth = length(l) / far_clip;  // linear depth of the current pixel

    // find a tangent T and binormal B such that T, B and L are orthogonal to each other
    // there's an infinite set of possible Ts and Bs, but we can choose them arbitrarily
    // because sampling on unit disk is symmetrical (T and B define the axes of the disk)
    // just pick a vector U that's not collinear with L, then cross(U, L) will give us T

    vec3 U = mix(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), step(abs(L.y), 0.999));
    vec3 T = normalize(cross(U, -L));
    vec3 B = normalize(cross(-L, T));

    // option 1: generate 16 random Poisson samples for each pixel (very slow at runtime)
    // const int n_samples = 16;
    // vec2 samples[16];
    // vec3 scale = abs(l);
    // float seed = min3(scale) / max3(scale);
    // PoissonSampleDisk(seed, samples);

    // option 2: use a pre-computed Poisson disk of 16 samples (much faster)
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
    //return total_depth/n_blocker;
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


layout(location = 0) in _vtx {
    in vec3 _position;
    in vec3 _normal;
    in vec2 _uv;
};

layout(location = 0) out vec4 color;

//lights data
layout(location = 932)uniform int numOfLights;
layout(binding = 7) uniform sampler2D lightsTex;
#define QUAD_LIGHT 0
#define SPHERE_LIGHT 1
#define DISTANT_LIGHT 2

void main() {
      Pixel px;
    
      px._position = _position;
      px._normal   = _normal;
      px._uv       = _uv;
      px._has_tbn  = false;
 
      InitPixel(px, cameraviewpos);
      vec3 Lo = vec3(0.0);
   

      Lo += EvaluateIBL(px) * 11.5;
      Lo+=EvaluateADL( px,vec3(1.0,0,0), 1.0f)*vec3(1.0,1.0,1.0)*10;
      //Lo+=EvaluateADL( px,vec3(-1.0,0,0), 1.0f)*vec3(1.0,1.0,1.0)*10;
      //Lo+=EvaluateADL( px,vec3(0,1.0,0), 1.0f)*vec3(1.0,1.0,1.0)*10;
      //Lo+=EvaluateADL( px,vec3(0,-1.0,0), 1.0f)*vec3(1.0,1.0,1.0)*10;
      //Lo+=EvaluateADL( px,vec3(0,0,1.0), 1.0f)*vec3(1.0,1.0,1.0)*10;
      //Lo+=EvaluateADL( px,vec3(0,0,-1.0), 1.0f)*vec3(1.0,1.0,1.0)*10;
      for (int i = 0; i < numOfLights; i++)
      {
 
        // Fetch light Data
        vec3 position = texelFetch(lightsTex, ivec2(i * 5 + 0, 0), 0).xyz;
        vec3 emission = texelFetch(lightsTex, ivec2(i * 5 + 1, 0), 0).xyz;
        vec3 u        = texelFetch(lightsTex, ivec2(i * 5 + 2, 0), 0).xyz;
        vec3 v        = texelFetch(lightsTex, ivec2(i * 5 + 3, 0), 0).xyz;
        vec3 params   = texelFetch(lightsTex, ivec2(i * 5 + 4, 0), 0).xyz;
        float radius  = params.x;
        float area    = params.y;
        float type    = params.z;

        // Distant light
        if (type == DISTANT_LIGHT)
        {
            //Lo+=EvaluateADL( px, position, 1.0f)*emission;

        }
        // Intersect rectangular area light
        if (type == QUAD_LIGHT)
        {
            vec3 normal = normalize(cross(u, v));
           // Lo+=EvaluateASL(px, position, normal,15.0f,12.0f,30.0f)*emission;

        }


        //Point light
        if (type == SPHERE_LIGHT)
        {

            //Lo+=EvaluateAPL(px,  position, 225.0f, 0.09f, 0.032f, 1.0f)*emission;
         
        } 
        
      }
     //Lo =Reinhard(Lo);
      Lo =ApproxACES(Lo);
      //color = vec4((_normal+1)/2,1.0);
      color = vec4(Lo, 1.0);

}

#endif


