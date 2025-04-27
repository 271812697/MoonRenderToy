#version 460 core

// compute diffuse irradiance map from an HDR environment cubemap texture

#ifdef compute_shader

#define PI2      6.283185307179586

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
layout(binding = 0) uniform samplerCube environment_map;
layout(binding = 0, rgba16f) restrict writeonly uniform imageCube irradiance_map;

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

vec3 Tangent2World(vec3 N, vec3 v) {
    N = normalize(N);

    // choose the up vector U that does not overlap with N
    vec3 U = mix(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), step(abs(N.y), 0.999));
    vec3 T = normalize(cross(U, N));
    vec3 B = normalize(cross(N, T));
    return T * v.x + B * v.y + N * v.z;  // mat3(T, B, N) * v
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
// this way of sampling over the hemisphere is cosine-weighted, preciser and faster.
// for 2K resolution we only need ~ 8000 samples to achieve nice results.
vec3 CosineConvolution(vec3 N, uint n_samples) {
    vec3 irradiance = vec3(0.0);

    for (uint i = 0; i < n_samples; i++) {
        vec2 u = Hammersley2D(i, n_samples);
        vec3 L = CosineSampleHemisphere(u.x, u.y);
        L = Tangent2World(N, L);
        irradiance += texture(environment_map, L).rgb;
    }

    /* since the sampling is already cosine-weighted, we can directly sum up the retrieved texel
       values and divide by the total number of samples, there's no need to include a weight and
       then balance the result with a multiplier. If we multiply each texel by `NoL` and then
       double the result as we did in uniform sampling, we are essentially weighing the radiance
       twice, in which case the result irradiance map would be less blurred where bright pixels
       appear brighter and dark areas are darker, in fact many people were doing this wrong.
    */

    return irradiance / float(n_samples);
}

// convert a 2D texture coordinate st on a cubemap face to its equivalent 3D
// texture lookup vector v such that `texture(cubemap, v) == texture(face, st)`
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

void main() {
    ivec3 ils_coordinate = ivec3(gl_GlobalInvocationID);
    vec2 resolution = vec2(imageSize(irradiance_map));
    vec3 N = ILS2Cartesian(ils_coordinate, resolution);

    // here we present 3 different ways of computing diffuse irradiance map from an HDR
    // environment map, all 3 have considered the cosine term in the integral, and will
    // yield results that are hardly distinguishable. The last one uses cosine-weighted
    // sampling, it's a lot more performant and requires much fewer samples to converge.

    vec3 irradiance = CosineConvolution(N, 16384);

    imageStore(irradiance_map, ils_coordinate, vec4(irradiance, 1.0));
}

#endif