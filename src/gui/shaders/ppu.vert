#version 300 es
precision highp float;

uniform int u_scy;

in vec2 pos;

out vec2 v_coord;

#define SCREEN_W        160.0
#define SCREEN_H        144.0

#define BG_W            256.0
#define BG_H            256.0

void viewport()
{
    v_coord.x *= SCREEN_W / BG_W;
    v_coord.y *= SCREEN_H / BG_H;

    v_coord.x *= BG_W;
    v_coord.y *= BG_H;

    v_coord.y += float(u_scy);

    v_coord.x /= BG_W;
    v_coord.y /= BG_H;
}

void main()
{
    // map to 0.0 - 1.0
    v_coord.x = pos.x * 0.5 + 0.5;
    v_coord.y = pos.y * -0.5 + 0.5;

    // viewport();

    gl_Position = vec4(pos, 0.0f, 1.0f);
}
