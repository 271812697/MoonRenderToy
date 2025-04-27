#version 460 core


#ifdef vertex_shader
layout (std140, binding = 0) uniform EngineUBO
{
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
};
layout(location = 0) in vec3 position;
layout(location = 0) out vec3 _tex_coords;

void main() {

    _tex_coords = position;

    mat4 rectified_view = mat4(mat3(ubo_View));
    vec4 pos = ubo_Projection * rectified_view * vec4(position, 1.0);

    gl_Position = pos.xyww;

}

#endif


#ifdef fragment_shader
layout(location = 0) out vec4 color;
layout(location = 1) out vec4 bloom;
layout(location = 0) in vec3 _tex_coords;



layout(binding = 1) uniform samplerCube skybox;
layout(location = 0) uniform float exposure = 1.0;
layout(location = 1) uniform float lod = 0.0;

vec3 ApproxACES(vec3 radiance) {
    vec3 v = radiance * 0.6;
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((v * (a * v + b)) / (v * (c * v + d) + e), 0.0, 1.0);
}

vec3 Linear2Gamma(vec3 color) {
    return pow(color, vec3(1.0 / 2.2));  // component-wise
}


void main() {

    const float max_level = textureQueryLevels(skybox) - 1.0;
    vec3 irradiance = textureLod(skybox, _tex_coords, clamp(lod, 0.0, max_level)).rgb;
    //color = vec4(Linear2Gamma(ApproxACES(irradiance * exposure)), 1.0);
    color = vec4(irradiance * exposure, 1.0);
    bloom=vec4(0.0);
}

#endif

