#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
uniform vec3 iResolution;

#define RES 16

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


void mainImage(out vec4 fragColor, vec2 fragCoord )
{
    float r = pow(2., float(RES));
    vec2  p = floor(fragCoord.xy / iResolution.xy * r);
    fragColor = vec4(vec3(xor(p) / r), 1);
}

void main()
{
	mainImage(FragColor, TexCoord * iResolution.xy);
}
