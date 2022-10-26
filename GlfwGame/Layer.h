#pragma once
#include <utility>
#include <array>

#include "glad.h"
#include "glfw3.h"

// API Layer for GLFW stuff
class Layer
{
public:
    struct Key_State {
        // key currently being pressed
        bool pressed;
        // key pressed this frame
        bool just_pressed;
    };

    // access with: GLFW_KEY_X
    Key_State key_state(int key) const;

    // access with GLFW_MOUSE_BUTTON_X
    Key_State mouse_btn_state(int button) const;

    // convert from SCREEN COORDINATES to normalized opengl coords
    std::pair<float, float> SC_to_N(std::pair<float, float> screen_pos) const;

    // x and y in screen coordinates
    std::pair<float, float> mouse_pos_SC() const;

    // normalized for opengl
    std::pair<float, float> mouse_pos_N() const;

    // mouse scroll X and Y
    std::pair<float, float> mouse_scroll() const;

    // in SCREEN COORDINATES
    std::pair<int, int> window_size() const;
    // in PIXELS
    std::pair<int, int> framebuffer_size() const;

    static unsigned int compile_shader_from_file(int type, const char* path, const char* error_msg);

    static unsigned int compile_shader_program(const char* vertexShaderSource, const char* fragmentShaderSource, const char* name_for_error);

    int start();
    
    bool loop_continue();
    // swap buffers, calculate framerate, set title, manage key api...
    void end_of_loop();

    void clean_up();

private:
    // The action is one of GLFW_PRESS, GLFW_REPEAT or GLFW_RELEASE
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

    static void error_callback(int code, const char* description);

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos); // gets in SCREEN COORDINATES

    // window size in SCREEN COORDINATES
    static void window_size_callback(GLFWwindow* window, int width, int height);

    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods); // action can only be PRESS or RELEASE

    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

    void reset_keys();

    static void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam);

    static constexpr int m_SCR_WIDTH = 960; // In SCREEN COORDINATES
    static constexpr int m_SCR_HEIGHT = 600; // In SCREEN COORDINATES
    static constexpr int m_FRAMERATE = 60; // max frames per second
    static constexpr double m_MIN_SEC_PER_FRAME = 1.0 / m_FRAMERATE;

    static constexpr int m_TOTAL_KEYS = GLFW_KEY_LAST + 1;
    // Keys currently held down
    std::array<bool, m_TOTAL_KEYS> m_pressed_keys = { false };
    // pressed this frame
    std::array<bool, m_TOTAL_KEYS> m_keys_just_pressed = { false };

    GLFWwindow* m_window = NULL;

    GLFWcursor* m_cursor = NULL;

    // in SCREEN COORDINATES
    std::pair<float, float> m_mouse_pos_SC = { 0.0, 0.0 };
    std::pair<float, float> m_mouse_pos_N = { 0.0, 0.0 };

    std::pair<float, float> m_scroll = { 0.0f, 0.0f }; // mouse scroll, x and y

    static constexpr int m_TOTAL_MOUSE_BUTTONS = GLFW_MOUSE_BUTTON_LAST + 1;

    std::array<bool, m_TOTAL_MOUSE_BUTTONS> m_mouse_buttons_pressed = { false };
    std::array<bool, m_TOTAL_MOUSE_BUTTONS> m_mouse_buttons_just_pressed = { false };

    std::pair<int, int> m_window_size = { m_SCR_WIDTH, m_SCR_HEIGHT }; // window width and height in SCREEN COORDINATES
    std::pair<int, int> m_framebuffer_size = { 0, 0 }; // window width and height in PIXELS, first set in start(), then callback

    bool m_fullscreen = false; // if custom fullscreen or not

    double m_start_time = 0.0; // time when frame started
};