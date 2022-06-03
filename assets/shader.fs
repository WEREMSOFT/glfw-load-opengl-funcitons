mat2 rot(float x)
{
    return mat2(cos(x), sin(x), -sin(x), cos(x));
}

float sdBox(vec3 p, vec3 b)
{
    vec3 d = abs(p) - b;
    return min(max(d.x, max(d.y, d.z)), 0.0) + length(max(d, 0.0));
}

vec3 tri(vec3 x)
{
    return abs(2.0 * (x - floor(x + 0.5))) * 2.0 - 1.0;
}

float scale = 0.25;
vec3 texw;
float mapw;

float map(vec3 p)
{
    mapw = 0.0;

    p.x += sin(p.z * scale);
    p.y += cos(p.z * scale) * sin(p.z * scale);

    vec3 q = p;

    float d = 1000.0;

    vec3 s = vec3(1.0, 0.125, 0.125);

    float u = 1.0;

    float gt = p.z;

    q = tri(q * 0.125);

    const int n = 4;
    for (int i = 0; i < n; ++i)
    {

        float fi = float(i) / float(n - 1);

        q = abs(q) - u;

        q.xy *= rot(q.z + gt);

        float k = sdBox(q, s);

        if (k < d)
        {
            d = k;
            texw = q;
            mapw = float(i);
        }

        s = s.yzx;
        s *= 0.5;
        u *= 0.5;
    }

    d = max(d, 2.0 - length(p.xy));

    return d;
}

float trace(vec3 o, vec3 r)
{
    float t = 0.0;
    for (int i = 0; i < 64; ++i)
    {
        t += map(o + r * t) * 0.5;
    }
    return t;
}

vec3 textex(sampler2D channel, vec3 p)
{
    vec3 ta = texture(channel, p.xy).xyz;
    vec3 tb = texture(channel, p.yz).xyz;
    vec3 tc = texture(channel, p.xz).xyz;
    return (ta * ta + tb * tb + tc * tc) / 3.0;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord.xy / iResolution.xy;
    uv = uv * 2.0 - 1.0;
    uv.x *= iResolution.x / iResolution.y;
    uv *= 2.5;

    vec3 o = vec3(0.0, 0.0, iTime * 2.0);
    vec3 r = normalize(vec3(uv, 1.5));

    o.x = -sin(o.z * scale);
    o.y = -cos(o.z * scale) * sin(o.z * scale);

    r.xy *= rot(iTime * 0.2);

    float t = trace(o, r);
    vec3 w = o + r * t;
    float fd = map(w);

    vec3 tex1 = textex(iChannel0, texw);
    tex1 = 1.0 - pow(1.0 - tex1, vec3(2.0));
    vec3 tex2 = textex(iChannel1, texw);
    tex2 = 1.0 - pow(1.0 - tex2, vec3(2.0, 3.0, 2.0));

    vec3 tex = tex1;
    if (mapw > 2.0)
    {
        tex = tex2;
    }

    float fog = 1.0 / (1.0 + t * t * t * 0.01 + fd * 100.0);

    vec3 fc = tex * vec3(fog);

    fragColor = vec4(sqrt(fc), 1.0);
}