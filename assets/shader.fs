float hash21(vec2 p)
{
    p = fract(p * vec2(234.34, 435.345));
    p += dot(p, p + 34.23);
    return fract(p.x * p.y);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - .5 * iResolution.xy) / iResolution.y;
    vec3 col = vec3(0);

    uv += iTime * .2;

    uv *= 50.;
    vec2 gv = fract(uv) - .5;
    vec2 id = floor(uv);

    float n = hash21(id); // random number between 0 and 1;

    float width = 0.1;

    if (n < .5)
        gv.x *= -1.;

    float d = abs(abs(gv.x + gv.y) - .5);

    float mask = smoothstep(.01, -.01, d - width);

    col += mask;

    // if (gv.x > .48 || gv.y > .48)
    //     col = vec3(1, 0, 0);

    fragColor = vec4(col, 1.0);
}
