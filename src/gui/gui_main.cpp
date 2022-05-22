#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GLES3/gl3.h>
#include "types.hpp"
#include "core/gameboy.hpp"
#include "core/ppu.hpp"
#include "common/logging.hpp"
#include <unordered_set>

static void glfw_error_callback(int error, const char* description)
{
    UNREACHABLE("Glfw Error {}: {}\n", error, description);
}

s32 gui_main(gbemu::core::Gameboy& gb)
{
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit())
        return 1;

    GLFWwindow* window = glfwCreateWindow(256*2, 256*2, "Gameboy", NULL, NULL);
    // GLFWwindow* window = glfwCreateWindow(gbemu::core::SCREEN_WIDTH*3, gbemu::core::SCREEN_HEIGHT*3, "Gameboy", NULL, NULL);
    if (window == NULL)
        return 1;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    bool stepping = false;
    bool inBreak = false;
    std::unordered_set<s16> breakpoints;

    // breakpoints.insert(0x0100);
    // breakpoints.insert(0x0034);
    // breakpoints.insert(0x001A);


    s32 old_key_run = GLFW_RELEASE;
    s32 old_key_step = GLFW_RELEASE;

    u32 ctr = 0;

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        if (breakpoints.contains(gb.cpu()->regs().pc) || stepping)
        {
            gb.cpu()->setLogging(true);
            if (!inBreak)
            {
                LOG("PRESS A KEY\n");
                inBreak = true;
            }
            s32 key_run = glfwGetKey(window, GLFW_KEY_R);
            s32 key_step = glfwGetKey(window, GLFW_KEY_S);

            if (key_step == GLFW_RELEASE && old_key_step == GLFW_PRESS)
            {
                stepping = true;
                inBreak = false;
                gb.step();
            }

            if (key_run == GLFW_RELEASE && old_key_run == GLFW_PRESS)
            {
                stepping = false;
            }

            old_key_run = key_run;
            old_key_step = key_step;
        }
        else // runnning
        {
            gb.cpu()->setLogging(false);
            gb.step();
        }

        // Rendering
        if (gb.ppu()->newFrameAvailable())
        {
            s32 display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            gb.ppu()->render();
            glfwSwapBuffers(window);
        }
    }

    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}