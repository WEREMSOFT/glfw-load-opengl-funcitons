#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
uniform vec3 screenSize;

#define RES 2

float xor(vec2 p)
{
    float r = 0.;
   	for (float i = 1.; i <= 32.; ++i) {
        float d = pow(2., i);
    	vec2  s = floor(mod(p / d, 2.));
        r += ((s.x == s.y) ? 0. : 1.) * d;
	}    
    return r;
}


vec4 mainImage(vec2 fragCoord )
{
    float r = pow(2., float(RES));
    vec2  p = floor(fragCoord.xy / screenSize.xy * r);
    return vec4(vec3(xor(p) / r), 1);
}

void main()
{
	// linearly interpolate between both textures (80% container, 20% awesomeface)
	// FragColor = vec4(screenSize.xy/TexCoord, 0, 1.f);
    // FragColor = vec4(screenSize, 0, 1.f);
	FragColor = mainImage(TexCoord);
}
