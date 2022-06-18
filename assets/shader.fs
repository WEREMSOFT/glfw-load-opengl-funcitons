#define MAX_STEPS 100
#define MAX_DIST 100.
#define MIN_DIST .001

mat2 getRot(float angle)
{
    float s = sin(angle);
    float c = cos(angle);
    return mat2(c, -s, s, c);
}

#define PI 3.141516

float T = iTime;

float getDist(vec3 p)
{
    vec2 k = {1., .2};
    vec3 kp = {0., 1.5, 0.};
    vec3 pk = p - kp;

    // mat2 m = getRot(PI / 2);

    // pk.yz *= m;
    // m = getRot(T / 2);
    // pk.xz *= m;

    vec2 ck = vec2(length(pk.xz) - k.x, pk.y);

    float a = atan(pk.x, pk.z);

    ck *= getRot(a * 2.);

    ck.y = abs(ck.y) - .3;

    float dk = length(ck) - k.y;
    float df = p.y + 1.;
    return min(df, dk);
}

float rayMarch(vec3 ro, vec3 rd, float maxD)
{
    float dO = 0.;
    for (int i = 0; i < MAX_STEPS; i++)
    {
        vec3 p = ro + rd * dO;
        float dS = getDist(p);
        dO += dS;
        if (dO > maxD || dS < MIN_DIST)
            break;
    }
    return dO;
}

vec3 getNormal(vec3 p)
{
    vec2 e = vec2(0.01, 0.);

    float d = getDist(p);

    vec3 n = d - vec3(
                     getDist(p - e.xyy),
                     getDist(p - e.yxy),
                     getDist(p - e.yyx));
    return normalize(n);
}

float getLight(vec3 p)
{
    vec3 lp = vec3(-5., 5., 2.);
    vec3 ld = normalize(lp - p);
    float d = length(lp - p);
    float lD = rayMarch(p, ld, d);

    vec3 n = getNormal(p);

    float l = dot(ld, n);

    if (rayMarch(p + n * 0.1, ld, d) < d)
        l *= .1;

    return l;
}

vec3 getBg(vec3 rd)
{
    float k = rd.y * .5 + .5;

    vec3 col = mix(vec3(.2, .1, .1), vec3(.2, .5, 1.), k);
    return col;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{

    vec2 uv = (fragCoord - .5 * iResolution.xy) / iResolution.y;
    float id = uv.x + uv.y;
    T = iTime * id;
    // uv = fract(uv * ceil(iTime)) - .5;

    vec3 ro = vec3(0., 1. + (.5 * sin(iTime)), 6.);
    ro.xz *= getRot(iTime);

    vec3 rd = normalize(vec3(uv, 1.));
    rd.xz *= -getRot(iTime);
    float d = rayMarch(ro, rd, MAX_DIST);

    vec3 col = getBg(rd);

    vec3 p = ro + rd * d;
    vec3 r = reflect(rd, getNormal(p));
    vec4 refl = texture(iChannel4, r);
    if (d < MAX_DIST)
    {
        float spec = pow(max(0., r.y), 70.);
        col = vec3(getLight(p)) + getBg(r) * .7;
        col = refl.xyz * .5 + spec + col * .2; //   +spec; // + (refl.xyz * .1);
    }
    fragColor = vec4(col, 1.) + texture(iChannel4, p) * .5; // * .5 + vec4(col, 1.);
}