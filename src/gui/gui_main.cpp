#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GLES3/gl3.h>
#include "types.hpp"
#include "core/gameboy.hpp"
#include "core/ppu.hpp"
#include "core/disas.hpp"
#include "common/logging.hpp"
#include <unordered_set>
#include <tuple>
#include "core/opcode.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"

static void glfw_error_callback(int error, const char* description)
{
    UNREACHABLE("Glfw Error {}: {}\n", error, description);
}

static bool s_is_running = true;
static std::unordered_set<u16> s_breakpoints;

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
        s32 p1 = gb.mem()->read8(gbemu::core::P1_ADDR).value_or(0);
        if (ImGui::InputInt("P1", &p1, 1, 100, ImGuiInputTextFlags_CharsHexadecimal))
            p1 = std::clamp(p1, 0, 0xFF);
        gb.mem()->write8(gbemu::core::P1_ADDR, p1);

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
            drawOam(gb);
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
