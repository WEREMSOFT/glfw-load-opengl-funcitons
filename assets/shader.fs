#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
uniform vec3 iResolution;
uniform float iTime;
uniform vec4 iMouse;

/// paste your code here
  // "Math Zoo - Alien Orb" 
// by Martijn Steinrucken aka BigWings/The Art of Code - 2022
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
// 
// Email: countfrolic@gmail.com
// Twitter: @The_ArtOfCode
// YouTube: youtube.com/TheArtOfCodeIsCool
//
// This started out with me playing with Gyroids and it ended up with
// me discovering an alien lifeform that rolls through the vast deserts
// of some far-flung planet in a different galaxy.
//
// Don't let its beauty fool you. Any unsuspecting creature entering the cage
// attracted by the light show will meet an untimely death. When it gets
// zapped by the plasma at the center, the tumbler collapses and suffocates
// its stunned prey. 
// Its like one of those fly zappers, only much cooler!
// To date, the plasma tumbler is the only creature we know of that uses 
// a plasma as part of its survival strategy.
//
// .. and I discovered all of that while playing with ShaderToy ;)

#define S smoothstep
#define AA 2
#define T iTime*.1
#define TAU 6.283185

#define MAX_STEPS 300
#define MAX_DIST 30.
#define SURF_DIST .001

mat2 Rot(float a) {
    float s = sin(a);
    float c = cos(a);
    return mat2(c, -s, s, c);
}

float smin( float a, float b, float k ) {
    float h = clamp( 0.5+0.5*(b-a)/k, 0., 1. );
    return mix( b, a, h ) - k*h*(1.0-h);
}

float Hash21(vec2 p) {
    p = fract(p*vec2(123.34,234.34));
    p += dot(p, p+23.43);
    return fract(p.x*p.y);
}

float Gyroid(vec3 p) {
    float scale = 10.;
    vec3 p2 = p*scale;
    p2.xy *= Rot(T);
    return (abs(dot(sin(p2), cos(p2.zxy)))-.4)/scale;
}

float sabs(float x, float k) {
    return sqrt(x*x+k);
}

float GetDist(vec3 p) {
    
    float sphere = abs(length(p)-1.)-.03;

    float d=smin(sphere, Gyroid(p)*.7, -.03);
    float ground = p.y+1.+S(.01, -.01, d)*.1;
   
    float x = p.x;
    p.x += T*1.3;
   
    vec3 p2 = p*5.;
    
    float wake = S(.4, .0, abs(p.z));
    wake *= S(0., -1., x);
	float gyroid = (sabs(dot(sin(p2), cos(p2.zxy)),wake)-.4)/10.;
    
    ground += gyroid;
    d = min(d, ground*.5);
    
    return d;
}

vec2 RayMarch(vec3 ro, vec3 rd) {
	float dO=0.;
    float dM=MAX_DIST;
    
    for(int i=0; i<MAX_STEPS; i++) {
    	vec3 p = ro + rd*dO;
        float dS = GetDist(p);
        if(dS<dM) dM = dS;
        dO += dS;
        if(dO>MAX_DIST || abs(dS)<SURF_DIST) break;
    }
    
    return vec2(dO, dM);
}

vec3 GetNormal(vec3 p) {
	float d = GetDist(p);
    vec2 e = vec2(.001, 0);
    
    vec3 n = d - vec3(
        GetDist(p-e.xyy),
        GetDist(p-e.yxy),
        GetDist(p-e.yyx));
    
    return normalize(n);
}

vec3 R(vec2 uv, vec3 p, vec3 l, float z) {
    vec3 f = normalize(l-p),
        r = normalize(cross(vec3(0,1,0), f)),
        u = cross(f,r),
        c = p+f*z,
        i = c + uv.x*r + uv.y*u,
        d = normalize(i-p);
    return d;
}

float GlitterLayer(vec2 p, float seed) {
    float t = iTime*3.+seed;
    vec2 id = floor(p);
    vec2 gv = fract(p)-.5;
    
    float n = Hash21(id);
    float x = fract(n*12.32);
    float y = fract(n*123.32);
    vec2 offs = vec2(x,y)-.5;
    
    float d = length(gv-offs*.8);
    float m = S(.2, .0, d);
    
    m *= pow(sin(t+n*6.2832)*.5+.5, 3.);
    return m;
}

vec3 RayPlane(vec3 ro, vec3 rd, vec3 p, vec3 n) {
    float t = max(0., dot(p-ro,n)/dot(rd, n));
    return ro + rd*t;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 m = iMouse.xy/iResolution.xy;
    float t = iTime*.1;
    vec3 col = vec3(0);
    
    vec3 ro = vec3(0, 1, -1)*2.;
    ro.yz *= Rot(-m.y*3.14+1.);
    ro.xz *= Rot(-m.x*6.2831+iTime*.05);
    ro.y = max(ro.y, -.9);
    
    for(int x=0; x<AA; x++) {
        for(int y=0; y<AA; y++) {
            vec2 offs = vec2(x, y)/float(AA) -.5;

            vec2 uv = (fragCoord+offs-.5*iResolution.xy)/iResolution.y;
            vec3 rd = R(uv, ro, vec3(0,0,0), 1.);

            float dist = RayMarch(ro, rd).x;
            
            vec3 lightPos = vec3(0);
            vec3 shadowPos = lightPos+normalize(ro-lightPos);
            vec3 p = ro + rd * dist;
            if(dist<MAX_DIST) {
                
                vec3 l = normalize(lightPos-p);
                vec3 n = GetNormal(p);

                float dif = clamp(dot(n, l)*.5+.5, 0., 1.);

                vec2 d = RayMarch(lightPos, l);

                float shadow = length(p)<1.03 ? 1. : S(SURF_DIST, SURF_DIST*20., d.y)*.6+.4;
                float falloff = min(1., 1./length(p.xz));               

                dif *= shadow*falloff*falloff;
                
                col += dif;
                
                // ground glitter
                if(p.y<-.9) {
                    vec2 st = p.xz;
                    float offs = dot(rd, vec3(10));

                    st.x += t*1.3;
                    float glitter = GlitterLayer(st*10., offs);
                    glitter += GlitterLayer(st*17.+3.1, offs);
                    glitter += GlitterLayer(st*23.+23.1, offs);
                    col += pow(glitter,5.)*falloff*shadow*shadow;
                }
            }
            // center light
            float centerDist = length(uv);
            float g = Gyroid(shadowPos);
            float light = S(0., .03, g);
            col += min(10., light*.02/max(centerDist,1e-3))*vec3(1.,.8,.9);
            
            // volumetric starburst
            float sb = max(0., Gyroid(normalize(RayPlane(ro, rd, vec3(0), normalize(ro)))));
            sb *= 3.*S(-.2,.1, centerDist-.4);
            col += sb;
            
            // SSS
            float sss = max(0., 1.-dot(uv, uv)*25.);
            sss *= sss;
            sss *= S(2.5,2., dist); // only on the front
            sss *= 1.-light*.5;
            vec3 P = p;
            
            float vein = S(-.01,.02, Gyroid(P+sin(P*30.+iTime)*.01)+.03);
            sss *= vein;
            col += sss*vec3(1,.1,.1);
            col += vec3(1,0,0)*(1.-vein)*sss;
        }
    }
    
    col /= float(AA*AA);
    
    float pulse = pow(sin(iTime)*.5+.5, 150.);
    t = iTime;
    float k = sin(t)+sin(t*5.)*.5+sin(t*17.)*.25+sin(t*37.)*.1;
    col *= 1.+k*.2;
    
    vec2 uv = (fragCoord-.5*iResolution.xy)/iResolution.y;
    col *= 1.-dot(uv,uv);
    
    col /= col+3.; col *= 3.; // tone mapping 
    
    fragColor = vec4(col,1.0);
}
/// leave this function as is
void main()
{
	mainImage(FragColor, TexCoord * iResolution.xy);
}
