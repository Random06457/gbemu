#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GLES3/gl3.h>
#include "types.hpp"
#include "core/gameboy.hpp"
#include "core/ppu.hpp"
#include "common/logging.hpp"

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

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Rendering

        s32 display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);


        while (!gb.ppu()->newFrameAvailable())
            gb.step();

        gb.ppu()->render();

        glfwSwapBuffers(window);
    }

    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}