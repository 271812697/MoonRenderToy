
#version 460 core
#pragma optimize(off)
#ifdef vertex_shader

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texCoords;

out vec2 TexCoords;

void main()
{
    gl_Position = vec4(position.x,position.y,0.0,1.0);
    TexCoords = texCoords;
}
#endif

#ifdef fragment_shader
in vec2 TexCoords;
out vec4 color;


float sdCircle(vec2 uv,float r){
    return length(uv)-r;
}
float sdRect(vec2 uv,float r){
    return max(abs(uv.x),abs(uv.y))-r;
}
float sdBox(vec2 p,vec2 b){
    vec2 d=abs(p)-b;
    return length(max(d,0))+min(max(d.x,d.y),0.0);

}

float smin(float a,float b,float k){
    float h = clamp(0.5+0.5*(a-b)/k,0.0,1.0);
    return mix(a,b,h)-k*h*(1.0-h);
}
float smax(float a,float b,float k){
    return -smin(-a,-b,k);
}
layout (location = 0) uniform vec2 X;
layout (location = 1) uniform vec2 Y;
void main()
{ 
    vec2 uv=TexCoords;
    color=vec4(0.0,0.0,0.0,0.0);
    float r=sdBox(uv-vec2((X[0]+X[1])/2,(Y[0]+Y[1])/2),vec2((X[1]-X[0])/2,(Y[1]-Y[0])/2));
    if(r<=0&&r>=-0.001)
    color = vec4(1.0,0.0,0.0,1.0);
    else
    discard;
   
}
#endif