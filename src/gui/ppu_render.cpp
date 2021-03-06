#include <GLES3/gl3.h>
#include "common/fs.hpp"
#include "common/logging.hpp"
#include "core/ppu.hpp"

namespace gbemu::core
{

static u32 compileShader(GLenum type, const std::filesystem::path& path)
{
    u32 shader = glCreateShader(type);
    auto shader_src = File::readAllText(path);
    if (!shader_src)
    {
        UNREACHABLE("failed to open {}\n", path.generic_string());
    }
    const char* src = shader_src.value().c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    s32 status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        char buff[0x100];
        glGetShaderInfoLog(shader, sizeof(buff), nullptr, buff);
        UNREACHABLE("Failed to compile {}:\n{}\n", path.generic_string(), buff);
    }
    return shader;
}

static u32 compileProgram()
{
    u32 program = glCreateProgram();
    u32 vert_shader = compileShader(GL_VERTEX_SHADER, "shaders/ppu.vert");
    u32 frag_shader = compileShader(GL_FRAGMENT_SHADER, "shaders/ppu.frag");

    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);
    glLinkProgram(program);
    s32 status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status)
    {
        char buff[0x100];
        glGetProgramInfoLog(program, sizeof(buff), nullptr, buff);
        UNREACHABLE("Failed to compile program:\n{}\n", buff);
    }
    return program;
}

void Ppu::render()
{
    static bool init = false;
    static int i = 0;
    // LOG("render {}\n", i++);
    // LOG("scy={}\n", m_scy);

    glClearColor(0.1, 0.1, 0.1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    static u32 tex = 0;
    static u32 program = 0;
    static u32 vbo = 0;
    // static u32 scy_loc = 0;
    static u32 tex_loc = 0;

    // size_t tex_w = 256;
    // size_t tex_h = 256;
    // u32* tex_data = m_bg_texture;
    size_t tex_w = SCREEN_WIDTH;
    size_t tex_h = SCREEN_HEIGHT;
    u32* tex_data = m_screen_texture;
    // size_t tex_w = 10;
    // size_t tex_h = 10;

    // std::fill_n(tex_data, tex_w * tex_h, 0xFFFFFFFF);

    // for (size_t x = 0; x < tex_w; x++)
    //     tex_data[x] = 0xFF0000FF;

    // for (size_t x = 0; x < tex_w; x++)
    //     tex_data[(tex_h-1)*tex_w + x] = 0xFFFF0000;

    // for (size_t y = 0; y < tex_h; y++)
    //     tex_data[y*tex_w] = 0xFF0000FF;

    // for (size_t y = 0; y < tex_h; y++)
    //     tex_data[y*tex_w+tex_w-1] = 0xFFFF0000;

    if (!init)
    {
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_w, tex_h, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, tex_data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        program = compileProgram();
        tex_loc = glGetUniformLocation(program, "u_texture");
        // scy_loc = glGetUniformLocation(program, "u_scy");

        u32 tex_coord_loc = glGetAttribLocation(program, "pos");

        static f32 tex_coords[] = {
            // clang-format off
            -1.0, -1.0,
            -1.0,  1.0,
            1.0,  1.0,

            -1.0, -1.0,
            1.0,  1.0,
            1.0,  -1.0,
            // clang-format on
        };

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(tex_coords), tex_coords,
                     GL_STATIC_DRAW);

        glEnableVertexAttribArray(tex_coord_loc);
        glVertexAttribPointer(tex_coord_loc, 2, GL_FLOAT, false,
                              2 * sizeof(f32), nullptr);

        init = true;
    }

    glUniform1f(tex_loc, tex);
    // glUniform1i(scy_loc, m_scy);

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_w, tex_h, GL_RGBA,
                    GL_UNSIGNED_BYTE, tex_data);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glUseProgram(program);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    m_new_frame_available = false;
}

}
