#version 330 core
out vec2 texCoord;
void main()
{
    texCoord = gl_MultiTexCoord0.xy;
}