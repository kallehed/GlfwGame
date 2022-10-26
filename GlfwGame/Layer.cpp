#include "Layer.h"
#include <fstream>

#include <iostream>
#include <chrono>
#include <thread>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Layer::Key_State Layer::key_state(int key) const
{
    return { m_pressed_keys[key], m_keys_just_pressed[key] };
}

Layer::Key_State Layer::mouse_btn_state(int button) const
{
    return { m_mouse_buttons_pressed[button], m_mouse_buttons_just_pressed[button] };
}

std::pair<float, float> Layer::SC_to_N(std::pair<float, float> screen_pos) const
{
    auto w_size = window_size();
    // normalize
    return { ((screen_pos.first / w_size.first) - 0.5f) * (2.0f),
             ((screen_pos.second / w_size.second) - 0.5f) * (-2.0f) };
}

// x and y in screen coordinates
std::pair<float, float> Layer::mouse_pos_SC() const {
    return m_mouse_pos_SC;
}

// normalized for opengl
std::pair<float, float> Layer::mouse_pos_N() const {
    return m_mouse_pos_N;
}

// mouse scroll X and Y
std::pair<float, float> Layer::mouse_scroll() const {
    return m_scroll;
}

// in SCREEN COORDINATES
std::pair<int, int> Layer::window_size() const {
    return m_window_size;
}
// in PIXELS
std::pair<int, int> Layer::framebuffer_size() const {
    return m_framebuffer_size;
}

unsigned int Layer::compile_shader_from_file(int type, const char* path, const char* error_msg)
{
    unsigned int shader = glCreateShader(type);

    // load from file
    char* shaderSource = new char[100000];
    {
        std::ifstream f(path);
        if (f) {
            f.getline(shaderSource, 100000, '\0');
        }
        else {
            std::cout << "ERROR::FILE_NOT_FOUND: " << path << "\n";
        }
    }

    glShaderSource(shader, 1, (&shaderSource), NULL);
    glCompileShader(shader);

    int success;
    char infoLog[512];

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << error_msg << infoLog << std::endl;
    }
    delete[] shaderSource;
    return shader;
}

unsigned int Layer::compile_shader_program(const char* vertexShaderSource, const char* fragmentShaderSource, const char* name_for_error)
{
    unsigned int vertexShader = compile_shader_from_file(GL_VERTEX_SHADER, vertexShaderSource, "ERROR::SHADER::VERTEX::COMPILATION_FAILED ");

    // fragment shader
    unsigned int fragmentShader = compile_shader_from_file(GL_FRAGMENT_SHADER, fragmentShaderSource, "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED ");

    // shader program
    unsigned int shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int  success;
    char infoLog[512];

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::PROGRAM::SHADERPROGRAM::COMPILATION_FAILED, NAME: " << name_for_error << "\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int Layer::start()
{
    if (!glfwInit()) { std::cout << "ERROR::glfwInit() returned false\n"; }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
    //glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
    //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    //glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    //glfwWindowHint(GLFW_REFRESH_RATE, 60);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // window

    m_window = glfwCreateWindow(m_SCR_WIDTH, m_SCR_HEIGHT, "GLFWGame", NULL /*glfwGetPrimaryMonitor()*/, NULL);
    if (m_window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(m_window);

    glfwSetWindowUserPointer(m_window, this);

    //glfwSwapInterval(1); // turn of vsync, cuz it doesnt even work in fullscreen

    { // ICON IMAGE
        GLFWimage image;
        image.pixels = stbi_load("icon.png", &image.width, &image.height, 0, 4); //rgba channels
        glfwSetWindowIcon(m_window, 1, &image);

        stbi_image_free(image.pixels);
    }

    { // cursor IMAGEs
        GLFWimage image;
        image.pixels = stbi_load("cursor.png", &image.width, &image.height, 0, 4); //rgba channels 

        m_cursor = glfwCreateCursor(&image, 0, 0);

        glfwSetCursor(m_window, m_cursor);

        stbi_image_free(image.pixels);
    }

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) // glad
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    { // set framebuffer
        int fb_width, fb_height;
        glfwGetFramebufferSize(m_window, &fb_width, &fb_height);
        glViewport(0, 0, fb_width, fb_height);
        m_framebuffer_size = { fb_width, fb_height };
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glfwSetWindowOpacity(m_window, 1.0f);

    glfwSetErrorCallback(&Layer::error_callback);
    glfwSetFramebufferSizeCallback(m_window, &Layer::framebuffer_size_callback);
    glfwSetKeyCallback(m_window, &Layer::key_callback);
    glfwSetCursorPosCallback(m_window, &Layer::cursor_position_callback);
    glfwSetWindowSizeCallback(m_window, &Layer::window_size_callback);
    glfwSetMouseButtonCallback(m_window, &Layer::mouse_button_callback);
    glfwSetScrollCallback(m_window, &Layer::scroll_callback);
    {
        int data;
        glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &data);
        std::cout << "max unfiform 4vecs:" << data << '\n';
    }

    return 0;
}
bool Layer::loop_continue() {
    return !glfwWindowShouldClose(m_window);
}
// swap buffers, calculate framerate, set title, manage key api...
void Layer::end_of_loop()
{
    glfwSwapBuffers(m_window);
    reset_keys();
    glfwPollEvents(); // calls callbacks

    { // delay to get right FPS
        double time_passed = glfwGetTime() - m_start_time;
        double sleep_time = (m_MIN_SEC_PER_FRAME - time_passed) / 2.0; // ?????? divide by 2, framerate otherwise halved?

        std::this_thread::sleep_for(std::chrono::duration<double>(sleep_time));

        int live_framerate = int(1.0 / (glfwGetTime() - m_start_time)); // frames per second (should be 60)
        glfwSetWindowTitle(m_window, std::to_string(live_framerate).c_str());
    }

    // errors
    {
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR)
        {
            std::cout << "ERROR: " << err << '\n';
        }
    }

    // input
    {
        if (key_state(GLFW_KEY_DELETE).just_pressed) { // close window
            glfwSetWindowShouldClose(m_window, true);
        }
        if (key_state(GLFW_KEY_ESCAPE).just_pressed) {
            if (m_fullscreen) // if fullscren, make it NOT fullscreen
            {
                glfwSetWindowPos(m_window, 200, 200);
                glfwSetWindowSize(m_window, m_SCR_WIDTH, m_SCR_HEIGHT);
                glfwSetWindowAttrib(m_window, GLFW_DECORATED, GLFW_TRUE); // turn ON decoration
            }
            else {
                const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

                // custom fullscreen workaround
                glfwSetWindowAttrib(m_window, GLFW_DECORATED, GLFW_FALSE); // turn off decoration
                glfwSetWindowPos(m_window, 0, 0);
                glfwSetWindowSize(m_window, mode->width + 1, mode->height); // HACK, make width + 1 to trick windows into not making it wierd fullscren, just windowed: but BIG
            }
            m_fullscreen = !m_fullscreen;
        }
    }
    m_start_time = glfwGetTime();
}
void Layer::clean_up()
{
    glfwDestroyCursor(m_cursor);
    glfwTerminate();
}

// The action is one of GLFW_PRESS, GLFW_REPEAT or GLFW_RELEASE
void Layer::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    //std::cout << "key: " << key << " Scancode: " << scancode << " S2: " << glfwGetKeyScancode(key) << "\n";
    if (action != GLFW_REPEAT && key >= 0)
    {
        Layer* layer = (Layer*)glfwGetWindowUserPointer(window);
        if (action == GLFW_PRESS)
        {
            layer->m_pressed_keys[key] = true;
            layer->m_keys_just_pressed[key] = true;
        }
        else
        { // GLFW_RELEASE EVENT it must be
            layer->m_pressed_keys[key] = false;
        }
    }
}
void Layer::error_callback(int code, const char* description)
{
    std::cout << "ERROR::GLFW::CODE: " << code << " Description: " << description << "\n";
}
void Layer::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    std::cout << "framebuffer changed: " << width << " " << height << "\n";
    glViewport(0, 0, width, height);
}
void Layer::cursor_position_callback(GLFWwindow* window, double xpos, double ypos) // gets in SCREEN COORDINATES
{
    //std::cout << "mouse: " << xpos << " Y: " << ypos << '\n';
    Layer* layer = (Layer*)glfwGetWindowUserPointer(window);
    layer->m_mouse_pos_SC = { (float)xpos, (float)ypos };
    layer->m_mouse_pos_N = layer->SC_to_N(layer->m_mouse_pos_SC);
    //std::cout << "mouse N: " << layer->m_mouse_pos_N.first << " Y: " << layer->m_mouse_pos_N.second << '\n';
}
// window size in SCREEN COORDINATES
void Layer::window_size_callback(GLFWwindow* window, int width, int height)
{
    std::cout << "window size changed: " << width << " " << height << "\n";
    Layer* layer = (Layer*)glfwGetWindowUserPointer(window);
    layer->m_window_size = { width, height };
}
void Layer::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) // action can only be PRESS or RELEASE
{
    Layer* layer = (Layer*)glfwGetWindowUserPointer(window);
    std::cout << "mouse btn: " << button << '\n';
    if (action == GLFW_PRESS) {
        layer->m_mouse_buttons_pressed[button] = true;
        layer->m_mouse_buttons_just_pressed[button] = true;
    }
    else { // has to be release
        layer->m_mouse_buttons_pressed[button] = false;
    }
}
void Layer::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    Layer* layer = (Layer*)glfwGetWindowUserPointer(window);
    std::cout << "SCROLL: " << xoffset << " y: " << yoffset << '\n';
    layer->m_scroll = { (float)xoffset, (float)yoffset };
}
void Layer::reset_keys() {
    m_keys_just_pressed = { false }; // set all to false
    m_mouse_buttons_just_pressed = { false };
    m_scroll = { 0.0f, 0.0f };
}

void APIENTRY Layer::glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}