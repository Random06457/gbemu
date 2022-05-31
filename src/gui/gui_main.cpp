#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GLES3/gl3.h>
#include <chrono>
#include <unordered_set>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"

#include "types.hpp"
#include "common/logging.hpp"
#include "core/cpu.hpp"
#include "core/disas.hpp"
#include "core/gameboy.hpp"
#include "core/memory.hpp"
#include "core/opcode.hpp"
#include "core/ppu.hpp"

static void glfw_error_callback(int error, const char* description)
{
    UNREACHABLE("Glfw Error {}: {}\n", error, description);
}

#define MMIO_REG_INPUT(name) mmioRegisterInput(gb, #name, gbemu::core::name##_ADDR)

static void mmioRegisterInput(gbemu::core::Gameboy& gb, const char* name, u16 addr)
{
    s32 reg = gb.mem()->read8(addr).value_or(0);
    if (ImGui::InputInt(name, &reg, 1, 100, ImGuiInputTextFlags_CharsHexadecimal))
        reg = std::clamp(reg, 0, 0xFF);
    gb.mem()->write8(gbemu::core::P1_ADDR, reg);
}


static bool s_is_running = true;
static std::unordered_set<u16> s_breakpoints;

static void drawAudio(gbemu::core::Gameboy& gb)
{
    if (ImGui::BeginTabItem("Audio"))
    {
        MMIO_REG_INPUT(NR10);
        MMIO_REG_INPUT(NR11);
        MMIO_REG_INPUT(NR12);
        MMIO_REG_INPUT(NR13);
        MMIO_REG_INPUT(NR14);

        ImGui::NewLine();

        MMIO_REG_INPUT(NR21);
        MMIO_REG_INPUT(NR22);
        MMIO_REG_INPUT(NR23);
        MMIO_REG_INPUT(NR24);

        ImGui::NewLine();

        MMIO_REG_INPUT(NR31);
        MMIO_REG_INPUT(NR32);
        MMIO_REG_INPUT(NR33);
        MMIO_REG_INPUT(NR34);

        ImGui::NewLine();

        MMIO_REG_INPUT(NR41);
        MMIO_REG_INPUT(NR42);
        MMIO_REG_INPUT(NR43);
        MMIO_REG_INPUT(NR44);

        ImGui::EndTabItem();
    }
}

static void drawDisassembly(gbemu::core::Gameboy& gb)
{
    ImGui::Text("Disassembly");
    // ImGui::BeginChild("Disassembly");

    u16 addr = gb.cpu()->regs().pc;

    u8 buff[3] = { 0 };

    for (size_t i = 0; i < 10; i++)
    {
        buff[0] = gb.mem()->read8(addr).value_or(0);

        size_t op_size = gbemu::core::Disas::opcodeSize(buff[0]);

        for (size_t j = 1; j < op_size; j++)
            buff[j] = gb.mem()->read8(addr+j).value_or(0);

        auto str = gbemu::core::Disas::isValidOpcode(buff[0])
            ? gbemu::core::Disas::disassemble(buff, sizeof(buff))
            : fmt::format("INVALID({:02X})", buff[0]);

        ImGui::Text("%04X: %s", addr, str.c_str());

        addr += gbemu::core::Disas::opcodeSize(buff[0]);
    }

    // ImGui::EndChild();
}

static void drawBreakpoints()
{
    // ImGui::NextColumn();

    // ImGui::BeginChild("Breakpoints");

    static std::vector<u16> addresses;
    static std::vector<std::string> names;
    // require because the result of c_str() is not valid past the string's lifespan
    static std::vector<const char*> names_cstr;

    static s32 bp_input;

    static s32 selected_item = -1;
    ImGui::ListBox("Breakpoints", &selected_item, names_cstr.data(), names.size());


    if (ImGui::InputInt("Address", &bp_input, 1, 100, ImGuiInputTextFlags_CharsHexadecimal))
        bp_input = std::clamp(bp_input, 0, 0xFFFF);


    if (ImGui::Button("Add"))
    {
        s_breakpoints.insert(bp_input);

        addresses.clear();
        names.clear();
        names_cstr.clear();
        for (auto& bp :  s_breakpoints)
        {
            addresses.push_back(bp);
            names.push_back(fmt::format("{:04X}", bp));
        }
        for (auto& str : names)
            names_cstr.push_back(str.c_str());
    }

    ImGui::SameLine();

    if (selected_item >= 0 && ImGui::Button("Remove"))
    {
        u16 addr = addresses[selected_item];
        s_breakpoints.erase(addr);
        addresses.erase(addresses.begin() + selected_item);
        names.erase(names.begin() + selected_item);
        names_cstr.erase(names_cstr.begin() + selected_item);
        selected_item = -1;
    }



    // ImGui::EndChild();
}

static void drawJoypad(gbemu::core::Gameboy& gb)
{
    if (ImGui::BeginTabItem("Joypad"))
    {
        MMIO_REG_INPUT(P1);

        ImGui::EndTabItem();
    }
}

static void drawPpu(gbemu::core::Gameboy& gb)
{
    if (ImGui::BeginTabItem("PPU"))
    {
        MMIO_REG_INPUT(SCX);
        MMIO_REG_INPUT(SCY);
        MMIO_REG_INPUT(WX);
        MMIO_REG_INPUT(WY);
        MMIO_REG_INPUT(LY);
        MMIO_REG_INPUT(LYC);

        ImGui::Text("LCDC : 0x%02X", gb.ppu()->lcdc().raw);
        ImGui::Text("BG and Win Enabled : %s", gb.ppu()->lcdc().bg_and_window_enable ? "True" : "False");
        ImGui::Text("Win Enabled : %s", gb.ppu()->lcdc().window_enable ? "True" : "False");
        ImGui::Text("OBJ Size : 8x%d", gb.ppu()->lcdc().obj_size == 0 ? 8 : 16);
        ImGui::Text("OBJ Enabled : %s", gb.ppu()->lcdc().obj_enable ? "True" : "False");

        ImGui::EndTabItem();
    }
}

static void drawOam(gbemu::core::Gameboy& gb)
{
    if (ImGui::BeginTabItem("OAM"))
    {
        ImGui::BeginChild("OAM");

        auto oam_table = gb.ppu()->oam();

        for (size_t i = 0; i < 40; i++)
        {
            ImGui::Text("X=0x%X; Y=0x%X; Tile=0x%X; Pri=%d; FlipX=%d; FlipY = %d; Pal=%d",
                oam_table[i].x, oam_table[i].y, oam_table[i].tile, oam_table[i].bg_and_window_over_obj,
                oam_table[i].flip_x, oam_table[i].flip_y, oam_table[i].dmg_palette);
        }

        ImGui::EndChild();

        ImGui::EndTabItem();
    }
}

static void drawCPU(gbemu::core::Gameboy& gb)
{
    if (ImGui::BeginTabItem("CPU"))
    {
        static auto last = std::chrono::system_clock::now();
        auto now = std::chrono::system_clock::now();
        auto frame_time = std::chrono::duration<f64>(now - last).count();
        static size_t frame_counter = 0;
        frame_counter++;
        static f64 fps = 0.0;
        if (frame_time >= 1.0)
        {
            last = now;
            fps = frame_counter / frame_time;
            frame_counter = 0;
        }

        ImGui::Text("%.2f FPS", fps);

        if (ImGui::Button(s_is_running ? "Pause" : "Run"))
            s_is_running = !s_is_running;

        ImGui::SameLine();

        if (ImGui::Button("Step"))
            gb.step();

        auto& regs = gb.cpu()->regs();
        int a = regs.a;
        int b = regs.b;
        int c = regs.c;
        int d = regs.d;
        int e = regs.e;
        int f = regs.f;
        int bc = regs.bc;
        int de = regs.de;
        int hl = regs.hl;
        int af = regs.af;
        int sp = regs.sp;
        int pc = regs.pc;
        bool flag_z = regs.flags.z;
        bool flag_n = regs.flags.n;
        bool flag_h = regs.flags.h;
        bool flag_c = regs.flags.c;

        if (ImGui::InputInt("A", &a, 1, 100, ImGuiInputTextFlags_CharsHexadecimal))
            regs.a = std::clamp(a, 0, 0xFF);
        if (ImGui::InputInt("B", &b, 1, 100, ImGuiInputTextFlags_CharsHexadecimal))
            regs.b = std::clamp(b, 0, 0xFF);
        if (ImGui::InputInt("C", &c, 1, 100, ImGuiInputTextFlags_CharsHexadecimal))
            regs.c = std::clamp(c, 0, 0xFF);
        if (ImGui::InputInt("D", &d, 1, 100, ImGuiInputTextFlags_CharsHexadecimal))
            regs.d = std::clamp(d, 0, 0xFF);
        if (ImGui::InputInt("E", &e, 1, 100, ImGuiInputTextFlags_CharsHexadecimal))
            regs.e = std::clamp(e, 0, 0xFF);
        if (ImGui::InputInt("F", &f, 1, 100, ImGuiInputTextFlags_CharsHexadecimal))
            regs.f = std::clamp(f, 0, 0xFF);

        ImGui::NextColumn();

        if (ImGui::InputInt("BC", &bc, 1, 100, ImGuiInputTextFlags_CharsHexadecimal))
            regs.bc = std::clamp(bc, 0, 0xFF);
        if (ImGui::InputInt("DE", &de, 1, 100, ImGuiInputTextFlags_CharsHexadecimal))
            regs.de = std::clamp(de, 0, 0xFF);
        if (ImGui::InputInt("HL", &hl, 1, 100, ImGuiInputTextFlags_CharsHexadecimal))
            regs.hl = std::clamp(hl, 0, 0xFF);
        if (ImGui::InputInt("AF", &af, 1, 100, ImGuiInputTextFlags_CharsHexadecimal))
            regs.af = std::clamp(af, 0, 0xFF);
        if (ImGui::InputInt("SP", &sp, 1, 100, ImGuiInputTextFlags_CharsHexadecimal))
            regs.sp = std::clamp(sp, 0, 0xFF);
        if (ImGui::InputInt("PC", &pc, 1, 100, ImGuiInputTextFlags_CharsHexadecimal))
            regs.pc = std::clamp(pc, 0, 0xFF);

        ImGui::NewLine();

        if (ImGui::Checkbox("Z", &flag_z))
            regs.flags.z = flag_z;
        ImGui::SameLine();
        if (ImGui::Checkbox("N", &flag_n))
            regs.flags.n = flag_n;
        ImGui::SameLine();
        if (ImGui::Checkbox("H", &flag_h))
            regs.flags.h = flag_h;
        ImGui::SameLine();
        if (ImGui::Checkbox("C", &flag_c))
            regs.flags.c = flag_c;

        drawDisassembly(gb);
        drawBreakpoints();

        ImGui::EndTabItem();
    }
}

static void drawImGui(gbemu::core::Gameboy& gb)
{
    if (ImGui::Begin("Main"))
    {
        if (ImGui::BeginTabBar("Debug"))
        {
            drawCPU(gb);
            drawJoypad(gb);
            drawPpu(gb);
            drawOam(gb);
            drawAudio(gb);
        }
        ImGui::EndTabBar();
    }
    ImGui::End();

}

GLFWwindow* g_window;

s32 gui_main(gbemu::core::Gameboy& gb)
{
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit())
        return 1;

    // g_window = glfwCreateWindow(256*3, 256*3, "Gameboy", NULL, NULL);
    g_window = glfwCreateWindow(gbemu::core::SCREEN_WIDTH*5, gbemu::core::SCREEN_HEIGHT*5, "Gameboy", NULL, NULL);
    if (g_window == NULL)
        return 1;

    glfwMakeContextCurrent(g_window);
    glfwSwapInterval(1); // Enable vsync

    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(g_window, true);
    ImGui_ImplOpenGL3_Init("#version 130");


    // Main loop
    while (!glfwWindowShouldClose(g_window))
    {
        glfwPollEvents();

        if (s_breakpoints.contains(gb.cpu()->regs().pc))
            s_is_running = false;

        if (s_is_running)
            gb.step();

        // Rendering (always render if we're not running)
        if (gb.ppu()->newFrameAvailable() || !s_is_running)
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            drawImGui(gb);

            ImGui::Render();

            s32 display_w, display_h;
            glfwGetFramebufferSize(g_window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            gb.ppu()->render();

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(g_window);
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Cleanup
    glfwDestroyWindow(g_window);
    glfwTerminate();

    return 0;
}
