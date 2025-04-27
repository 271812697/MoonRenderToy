#version 460 core
#pragma optimize(off)

// compute specular environment BRDF (multi-scattering LUT with energy compensation)

#ifdef compute_shader

#define PI       3.141592653589793
#define PI2      6.283185307179586
#define INV_PI   0.318309886183791  // 1 over PI
layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
layout(binding = 2, rgba16f) restrict writeonly uniform image2D BRDF_LUT;
float pow5(float x) { return x * x * x * x * x; }
// Smith's height-correlated visibility function (V = G / normalization term)
// Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"
float V_SmithGGX(float alpha, float NoV, float NoL) {
    float a2 = alpha * alpha;
    float GGXV = NoL * sqrt(NoV * NoV * (1.0 - a2) + a2);
    float GGXL = NoV * sqrt(NoL * NoL * (1.0 - a2) + a2);
    return 0.5 / (GGXV + GGXL);
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
// uniformly sample a point (also a direction vector) on the unit hemisphere
// the probability of a point being sampled is 1 / (2 * PI), i.e. unbiased
vec3 UniformSampleHemisphere(float u, float v) {
    float phi = v * PI2;
    float cos_theta = 1.0 - u;  // ~ [0, 1]
    float sin_theta = sqrt(1.0 - cos_theta * cos_theta);
    return vec3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);
}
// Charlie distribution function based on an exponentiated sinusoidal
// Estevez and Kulla 2017, "Production Friendly Microfacet Sheen BRDF"
float D_Charlie(float alpha, float NoH) {
    float inv_alpha = 1.0 / alpha;
    float cos2 = NoH * NoH;
    float sin2 = 1.0 - cos2;
    return (2.0 + inv_alpha) * pow(sin2, inv_alpha * 0.5) / PI2;
}
// Neubelt's smooth visibility function for use with cloth and velvet distribution
// Neubelt and Pettineo 2013, "Crafting a Next-gen Material Pipeline for The Order: 1886"
float V_Neubelt(float NoV, float NoL) {
    return 1.0 / (4.0 * (NoL + NoV - NoL * NoV));
}
vec3 IntegrateBRDF(float NoV, float roughness, uint n_samples) {
    float alpha = roughness * roughness;
    float inv_ns = 1.0 / float(n_samples);

    NoV = max(NoV, 1e-4);  // reduce artifact on the border
    vec3 N = vec3(0.0, 0.0, 1.0);
    vec3 V = vec3(sqrt(1.0 - NoV * NoV), 0.0, NoV);  // vec3(sin, 0, cos)

    float scale = 0.0;
    float bias = 0.0;

    for (uint i = 0; i < n_samples; i++) {
        vec2 u = Hammersley2D(i, n_samples);
        vec3 H = ImportanceSampleGGX(u.x, u.y, alpha);  // keep in tangent space
        precise vec3 L = 2 * dot(V, H) * H - V;  // need the precise qualifier

        // implicitly assume that all vectors lie in the X-Z plane
        float NoL = max(L.z, 0.0);
        float NoH = max(H.z, 0.0);
        float HoV = max(dot(H, V), 0.0);

        if (NoL > 0.0) {
           // float pdf = D_TRGGX(NoH, alpha) * NoH / (4.0 * HoV);
            float V = V_SmithGGX(alpha, NoV, NoL) * NoL * HoV / max(NoH, 1e-5);
           
            float Fc = pow5(1.0 - HoV);  // Fresnel F has been factorized out of the integral

            // scale += V * (1.0 - Fc);  // this only considers single bounce
             //bias += V * Fc;           // this only considers single bounce

            scale += V * Fc;  // take account of multi-scattering energy compensation
            bias += V;        // take account of multi-scattering energy compensation
        }
    }

    scale *= (4.0 * inv_ns);
    bias  *= (4.0 * inv_ns);


    // for cloth, write a single DG term in the 3rd channel of the BRDF LUT
    // this term is generated using uniform sampling so we need another run
    // see Estevez & Kulla 2017, "Production Friendly Microfacet Sheen BRDF"

    float cloth = 0.0;

    for (uint i = 0; i < n_samples; i++) {
        vec2 u = Hammersley2D(i, n_samples);
        vec3 H = UniformSampleHemisphere(u.x, u.y);
        precise vec3 L = 2 * dot(V, H) * H - V;

        float NoL = max(L.z, 0.0);
        float NoH = max(H.z, 0.0);
        float HoV = max(dot(H, V), 0.0);

        if (NoL > 0.0) {
            float V = V_Neubelt(NoV, NoL);
            float D = D_Charlie(alpha, NoH);
            cloth += V * D * NoL * HoV;
        }
    }

    // integrate 1 over the hemisphere yields 2PI, hence the PDF of uniform sampling
    // is 1 over 2PI, so now we compensate 2PI back (4 comes from the Jacobian term)
    cloth *= (4.0 * PI2 * inv_ns);

    return vec3(scale, bias, cloth);
}

void main() {
    vec2 resolution = vec2(imageSize(BRDF_LUT));
    float cosine    = float(gl_GlobalInvocationID.x) / resolution.x;
    float roughness = float(gl_GlobalInvocationID.y) / resolution.y;

    vec3 texel = IntegrateBRDF(cosine, roughness, 2048);
    imageStore(BRDF_LUT, ivec2(gl_GlobalInvocationID.xy), vec4(texel, 0.0));
}

#endif
