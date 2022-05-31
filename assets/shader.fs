#version 330 core
in vec2 texCoord;

main()
{
    float x1 = texCoord.s;
    float x2 = texCoord.t;

    gl_FragColor = vec4(x1, x2, 0.0, 1.0);
}
