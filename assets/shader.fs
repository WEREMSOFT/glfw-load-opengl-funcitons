// Code by Flopine

// Thanks to wsmind, leon, XT95, lsdlive, lamogui, 
// Coyhot, Alkama,YX, NuSan, slerpy, wwrighter 
// BigWings, FabriceNeyret and Blackle for teaching me

// Thanks LJ for giving me the spark :3

// Thanks to the Cookie Collective, which build a cozy and safe environment for me 
// and other to sprout :)  
// https://twitter.com/CookieDemoparty


// Based on BigWings tuts ~ https://youtu.be/2dzJZx0yngg

#define PI acos(-1.)
#define rot(a) mat2(cos(a),sin(a),-sin(a),cos(a))

#define pal(t,c) (vec3(0.5)+vec3(0.5)*cos(2.*PI*(c*t+vec3(0.,0.5,0.63))))
#define swi(sp) floor(sin(fract(iTime*sp)*2.*PI))+1.

float box2d (vec2 p, vec2 c)
{
    vec2 q = abs(p)-c;
    return min(0.,max(q.x,q.y))+length(max(q,0.));
}

float a;
float torus (vec3 p, vec2 rs)
{
    float s = length(p.xz)-rs.x, b = max(abs(p.x),abs(p.z))-rs.x;
    vec2 cp = mix(vec2(b,p.y),vec2(s,p.y),clamp(swi(0.2),0.,1.));
    a = atan(p.z,p.x);
    cp *= rot(a+iTime);
    cp.y = abs(abs(cp.y)-.6)-.3;
    
    float d = box2d(cp,vec2(rs.y,(rs.y+sin(a)/PI)*.35))-0.03;
    return d;
}

float SDF (vec3 p)
{
    p.yz *= rot(-atan(1./sqrt(2.)));
    p.xz *= rot(PI/4.);
    
    return torus(p,vec2(2.5,0.3));
}

vec3 getnorm (vec3 p)
{
    vec2 eps = vec2(0.001,0.);
    return normalize(SDF(p)-vec3(SDF(p-eps.xyy),SDF(p-eps.yxy),SDF(p-eps.yyx)));
}

float spec (vec3 n, vec3 l, vec3 rd)
{
    vec3 h = normalize(l-rd);
    return pow(max(dot(n,h),0.), 25.);
}

float AO (float eps, vec3 p, vec3 n)
{return SDF(p+eps*n)/eps;}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = (2.*fragCoord-iResolution.xy)/iResolution.y;
    vec3 ro=vec3(uv*3.5,-30.),rd=vec3(0.,0.,1.),p=ro,
    col=vec3(0.95,0.9,0.8),l=vec3(1.,2.,-2.);

    bool hit=false;
    
    for(float i=0.; i<60.; i++)
    {
        float d = SDF(p);
        if (d<0.01)
        {
            hit=true;break;
        }
        p += d*rd*0.65;
    }

    if (hit)
    {
        vec3 n = getnorm(p);
        float light = dot(n,normalize(l))*.5+.5, ao=AO(0.1,p,n)+AO(0.45,p,n)+AO(0.8,p,n);
        col = mix(vec3(0.,0.,0.2),pal(a/PI,vec3(2.)),light)*ao/3.+spec(n,l,rd);
    }
    
    // Output to screen
    fragColor = vec4(sqrt(col),1.0);
}