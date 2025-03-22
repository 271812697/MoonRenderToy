#version 330
#ifdef vertex_shader

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoords;

out vec2 TexCoords;

void main()
{
	gl_Position = vec4(position.x, position.y, -1.0, 1.0);
	TexCoords = texCoords;
}
#endif

#ifdef fragment_shader
out vec4 color;
in vec2 TexCoords;

uniform sampler2D imgTex;

void main()
{
	color = texture(imgTex, TexCoords);
}
#endif