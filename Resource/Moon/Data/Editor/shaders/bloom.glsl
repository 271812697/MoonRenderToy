#version 460 core

// simple bloom effect in compute shader (1 downsample + 6 Gaussian blur passes + 1 upsample)

// for image load store operations, reading from any texel outside the boundaries will return 0
// and writing to any texel outside the boundaries will do nothing, so we can safely ignore all
// boundary checks. This shader can be slightly improved by using shared local storage to cache
// the fetched pixels, but the performance gain is small since imageLoad() is already very fast

// caution: please do not blur the alpha channel!



#ifdef compute_shader

#define k11x11
#ifndef _GAUSSIAN_H
#define _GAUSSIAN_H

#ifdef k9x9

// precomputed 9x9 Gaussian blur filter (sigma ~= 1.75)
const float weight[5] = float[] (
    0.2270270270,  // offset 0
    0.1945945946,  // offset +1 -1
    0.1216216216,  // offset +2 -2
    0.0540540541,  // offset +3 -3
    0.0162162162   // offset +4 -4
);

#endif

#ifdef k11x11

// precomputed 11x11 Gaussian blur filter (sigma = 4)
const float weight[6] = float[] (
    0.1198770,  // offset 0
    0.1161890,  // offset +1 -1
    0.1057910,  // offset +2 -2
    0.0904881,  // offset +3 -3
    0.0727092,  // offset +4 -4
    0.0548838   // offset +5 -5
);

#endif

#ifdef k13x13

// precomputed 13x13 Gaussian blur filter (sigma = 4)
const float weight[7] = float[] (
    0.1112200,  // offset 0
    0.1077980,  // offset +1 -1
    0.0981515,  // offset +2 -2
    0.0839534,  // offset +3 -3
    0.0674585,  // offset +4 -4
    0.0509203,  // offset +5 -5
    0.0361079   // offset +6 -6
);

#endif

#ifdef k15x15

// precomputed 15x15 Gaussian blur filter (sigma = 4)
const float weight[8] = float[] (
    0.1061150,  // offset 0
    0.1028510,  // offset +1 -1
    0.0936465,  // offset +2 -2
    0.0801001,  // offset +3 -3
    0.0643623,  // offset +4 -4
    0.0485832,  // offset +5 -5
    0.0344506,  // offset +6 -6
    0.0229491   // offset +7 -7
);

#endif

#ifdef k19x19

// precomputed 19x19 Gaussian blur filter (sigma = 5)
const float weight[10] = float[] (
    0.0846129,  // offset 0
    0.0829375,  // offset +1 -1
    0.0781076,  // offset +2 -2
    0.0706746,  // offset +3 -3
    0.0614416,  // offset +4 -4
    0.0513203,  // offset +5 -5
    0.0411855,  // offset +6 -6
    0.0317562,  // offset +7 -7
    0.0235255,  // offset +8 -8
    0.0167448   // offset +9 -9
);

#endif

#endif


layout(local_size_x = 32, local_size_y = 18, local_size_z = 1) in;  // fit 16:9 aspect ratio

layout(binding = 0, rgba16f) uniform image2D ping;
layout(binding = 1, rgba16f) uniform image2D pong;

layout(location = 0) uniform bool horizontal;

void GaussianBlurH(const ivec2 coord) {
    vec3 color = imageLoad(ping, coord).rgb * weight[0];
    for (int i = 1; i < 6; i++) {
        ivec2 offset = ivec2(i, 0);
        color += imageLoad(ping, coord + offset).rgb * weight[i];
        color += imageLoad(ping, coord - offset).rgb * weight[i];
    }
    imageStore(pong, coord, vec4(color, 1.0));
}

void GaussianBlurV(const ivec2 coord) {
    vec3 color = imageLoad(pong, coord).rgb * weight[0];
    for (int i = 1; i < 6; i++) {
        ivec2 offset = ivec2(0, i);
        color += imageLoad(pong, coord + offset).rgb * weight[i];
        color += imageLoad(pong, coord - offset).rgb * weight[i];
    }
    imageStore(ping, coord, vec4(color, 1.0));
}

void main() {
    ivec2 ils_coord = ivec2(gl_GlobalInvocationID.xy);

    if (horizontal) {
        GaussianBlurH(ils_coord);  // horizontal
    }
    else {
        GaussianBlurV(ils_coord);  // vertical
    }
}

#endif

