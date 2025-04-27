#version 460 core
#pragma optimize(off)
layout(std140, binding = 0) uniform EngineUBO
{
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
};

////////////////////////////////////////////////////////////////////////////////
#ifdef vertex_shader
layout(location = 0) in vec3 position;
void main() {
    gl_Position = ubo_Model * vec4(position, 1.0);  // keep in world space
}
#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef geometry_shader

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

layout(location = 0) out vec4 world_position;
layout(location = 250) uniform mat4 light_transform[6];

void main() {
    // render target is a cubemap texture, so we render 6 times, once for each face
    for (int face = 0; face < 6; ++face) {
        gl_Layer = face;
        uint index = uint(face);

        for (uint i = 0; i < 3; ++i) {
            world_position = gl_in[i].gl_Position;
            gl_Position = light_transform[index] * world_position;  // project into light frustum
            EmitVertex();
        }

        EndPrimitive();
    }
}

#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef fragment_shader
//layout(location = 0) out vec4 color;

layout(location = 0) in vec4 world_position;

layout(location = 1008) uniform vec3 light_pos;

void main() {
    gl_FragDepth = distance(world_position.xyz,light_pos) / 100.0;
    //color=vec4(0.0,0.0,0.0,0.0);
}

#endif
