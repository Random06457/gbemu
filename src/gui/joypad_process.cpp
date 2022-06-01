#include <GLFW/glfw3.h>
#include "core/int_controller.hpp"
#include "core/joypad.hpp"

extern GLFWwindow* g_window;

namespace gbemu::core
{

void Joypad::processInput()
{
    u8 old_p1 = m_p1.raw;

    if (m_p1.select_direction == 0 && m_p1.select_button == 1)
    {
        m_p1.up_select = glfwGetKey(g_window, GLFW_KEY_UP) == GLFW_RELEASE;
        m_p1.down_start = glfwGetKey(g_window, GLFW_KEY_DOWN) == GLFW_RELEASE;
        m_p1.right_a = glfwGetKey(g_window, GLFW_KEY_RIGHT) == GLFW_RELEASE;
        m_p1.left_b = glfwGetKey(g_window, GLFW_KEY_LEFT) == GLFW_RELEASE;
    }
    if (m_p1.select_button == 0 && m_p1.select_direction == 1)
    {
        m_p1.up_select =
            glfwGetKey(g_window, GLFW_KEY_RIGHT_CONTROL) == GLFW_RELEASE;
        m_p1.down_start = glfwGetKey(g_window, GLFW_KEY_SPACE) == GLFW_RELEASE;
        m_p1.right_a = glfwGetKey(g_window, GLFW_KEY_ENTER) == GLFW_RELEASE;
        m_p1.left_b =
            glfwGetKey(g_window, GLFW_KEY_RIGHT_SHIFT) == GLFW_RELEASE;
    }

    if ((old_p1 & ~m_p1.raw) & 0xF)
    {
        m_interrupts->requestInterrupt(InterruptType_Joypad);
    }
}

}
