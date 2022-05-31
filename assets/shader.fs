#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

void main()
{
	// linearly interpolate between both textures (80% container, 20% awesomeface)
	FragColor = vec4(TexCoord.xy, 0, 1.f);
}