#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
uniform vec3 iResolution;
uniform float iTime;

/// paste your code here
#define MAX_STEPS 100
#define MAX_DIST 100.
#define SURFACE_DIST .001

float GetDist(vec3 p){
    vec4 sphere = vec4(0,1,6,1);
    float dSphere = length(p-sphere.xyz)-sphere.w;
    float dPlane = p.y;
    float d = min(dSphere, dPlane);
    return d;
}

float RayMarch(vec3 ro, vec3 rd){
    float dO = 0.;
    for(int i=0; i<MAX_STEPS; i++) {
    	vec3 p = ro + dO*rd;
        float dS = GetDist(p);
        dO += dS;
        if(dS<SURFACE_DIST || dO>MAX_DIST) break;
    }
    return dO;
}

vec3 GetNormal(vec3 p){
    vec2 e = vec2(.01, 0);
    float d = GetDist(p);
    vec3 n = vec3(
    	d-GetDist(p-e.xyy),
        d-GetDist(p-e.yxy),
        d-GetDist(p-e.yyx)
    );
    
    return normalize(n);
}

float GetLight(vec3 p){
    vec3 lightPos = vec3(0, 5, 6);
    lightPos.xz += vec2(sin(0), cos(0))*2.;
    vec3 l = normalize(lightPos-p);
    vec3 n = GetNormal(p);
    
    float dif = clamp(dot(n, l), 0., 1.);
    float d = RayMarch(p+n*SURFACE_DIST*2., l);
    if (d<length(lightPos-p)) dif *= .1;
    return dif;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = (fragCoord-.5*iResolution.xy)/iResolution.y;

    vec3 col = vec3(0);
    
    vec3 ro = vec3(0,1,0);
    vec3 rd = normalize(vec3(uv.x, uv.y, 1));
    
    float d = RayMarch(ro, rd);
    
    vec3 p = ro + rd*d;
    
    float dif = GetLight(p);
    col = vec3(dif);

    fragColor = vec4(col,1.0);
}

/// leave this function as is
void main()
{
	mainImage(FragColor, TexCoord * iResolution.xy);
}
