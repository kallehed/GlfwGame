#include "glad.h"
#include "glfw3.h"

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>
#include <array>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// settings
constexpr int SCR_WIDTH = 960; // In SCREEN COORDINATES
constexpr int SCR_HEIGHT = 600; // In SCREEN COORDINATES
constexpr int FRAMERATE = 60; // max frames per second
constexpr double MIN_SEC_PER_FRAME = 1.0 / FRAMERATE;

unsigned int compile_shader_from_file(int type, const char *path, const char *error_msg)
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

unsigned int compile_shader_program(const char *vertexShaderSource, const char *fragmentShaderSource, const char *name_for_error)
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

struct Key_State {
    // key currently being pressed
    bool pressed;
    // key pressed this frame
    bool just_pressed;
};

// API Layer for GLFW stuff
class Layer
{
public:
    

    // access with: GLFW_KEY_X
    Key_State key_state(int key) const
    {
        return { m_pressed_keys[key], m_keys_just_pressed[key] };
    }

    // access with GLFW_MOUSE_BUTTON_X
    Key_State mouse_btn_state(int button) const
    {
        return { m_mouse_buttons_pressed[button], m_mouse_buttons_just_pressed[button] };
    }

    // convert from SCREEN COORDINATES to normalized opengl coords
    std::pair<float, float> SC_to_N(std::pair<float, float> screen_pos) const
    {
        auto w_size = window_size();
        // normalize
        return { ((screen_pos.first / w_size.first) - 0.5f) * (2.0f),
                 ((screen_pos.second / w_size.second) - 0.5f) * (-2.0f)};
    }

    // x and y in screen coordinates
    std::pair<float, float> mouse_pos_SC() const {
        return m_mouse_pos_SC;
    }

    // normalized for opengl
    std::pair<float, float> mouse_pos_N() const {
        return m_mouse_pos_N;
    }

    // mouse scroll X and Y
    std::pair<float, float> mouse_scroll() const {
        return m_scroll;
    }

    // in SCREEN COORDINATES
    std::pair<int, int> window_size() const {
        return m_window_size;
    }
    // in PIXELS
    std::pair<int, int> framebuffer_size() const {
        return m_framebuffer_size;
    }

private:
    // The action is one of GLFW_PRESS, GLFW_REPEAT or GLFW_RELEASE
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) 
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
    static void error_callback(int code, const char* description)
    {
        std::cout << "ERROR::GLFW::CODE: " << code << " Description: " << description << "\n";
    }
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
    {
        std::cout << "framebuffer changed: " << width << " " << height << "\n";
        glViewport(0, 0, width, height);
    }
    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) // gets in SCREEN COORDINATES
    {
        //std::cout << "mouse: " << xpos << " Y: " << ypos << '\n';
        Layer* layer = (Layer*)glfwGetWindowUserPointer(window);
        layer->m_mouse_pos_SC = { (float)xpos, (float)ypos };
        layer->m_mouse_pos_N = layer->SC_to_N(layer->m_mouse_pos_SC);
        //std::cout << "mouse N: " << layer->m_mouse_pos_N.first << " Y: " << layer->m_mouse_pos_N.second << '\n';
    }
    // window size in SCREEN COORDINATES
    static void window_size_callback(GLFWwindow* window, int width, int height)
    {
        std::cout << "window size changed: " << width << " " << height << "\n";
        Layer* layer = (Layer*)glfwGetWindowUserPointer(window);
        layer->m_window_size = { width, height };
    }
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) // action can only be PRESS or RELEASE
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
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
        Layer* layer = (Layer*)glfwGetWindowUserPointer(window);
        std::cout << "SCROLL: " << xoffset << " y: " << yoffset << '\n';
        layer->m_scroll = { (float)xoffset, (float)yoffset };
    }

public:
    int start()
    {
        if (!glfwInit()) { std::cout << "ERROR::glfwInit() returned false\n"; }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
        //glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
        //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        //glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
        //glfwWindowHint(GLFW_REFRESH_RATE, 60);

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        // window

        m_window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "GLFWGame", NULL /*glfwGetPrimaryMonitor()*/, NULL);
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
        int data;
        glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &data);
        std::cout << "max unfiform 4vecs:" << data << '\n';

        return 0;
    }
    bool loop_continue() {
        return !glfwWindowShouldClose(m_window);
    }
    // swap buffers, calculate framerate, set title, manage key api...
    void end_of_loop()
    {
        glfwSwapBuffers(m_window);
        reset_keys();
        glfwPollEvents(); // calls callbacks

        {
            double time_passed = glfwGetTime() - m_start_time;
            double sleep_time = (MIN_SEC_PER_FRAME - time_passed)/2.0 ; // ?????? divide by 2, framerate otherwise halved?

            std::this_thread::sleep_for(std::chrono::duration<double>(sleep_time));

            int live_framerate = int(1.0 / (glfwGetTime() - m_start_time)); // frames per second (should be 60)
            glfwSetWindowTitle(m_window, std::to_string(live_framerate).c_str());
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
                    glfwSetWindowSize(m_window, SCR_WIDTH, SCR_HEIGHT);
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
    void clean_up()
    {
        glfwDestroyCursor(m_cursor);
        glfwTerminate();
    }
protected:
    void reset_keys() {
        m_keys_just_pressed = { false }; // set all to false
        m_mouse_buttons_just_pressed = { false };
        m_scroll = { 0.0f, 0.0f };
    }
private:
    static constexpr int m_TOTAL_KEYS = GLFW_KEY_LAST + 1;
    // Keys currently held down
    std::array<bool, m_TOTAL_KEYS> m_pressed_keys = { false };
    // pressed this frame
    std::array<bool, m_TOTAL_KEYS> m_keys_just_pressed = { false };

    GLFWwindow *m_window = NULL;

    GLFWcursor *m_cursor = NULL;

    // in SCREEN COORDINATES
    std::pair<float, float> m_mouse_pos_SC = {0.0, 0.0};
    std::pair<float, float> m_mouse_pos_N = {0.0, 0.0};

    std::pair<float, float> m_scroll = { 0.0f, 0.0f }; // mouse scroll, x and y

    static constexpr int m_TOTAL_MOUSE_BUTTONS = GLFW_MOUSE_BUTTON_LAST + 1;

    std::array<bool, m_TOTAL_MOUSE_BUTTONS> m_mouse_buttons_pressed = { false };
    std::array<bool, m_TOTAL_MOUSE_BUTTONS> m_mouse_buttons_just_pressed = { false };

    std::pair<int, int> m_window_size = { SCR_WIDTH, SCR_HEIGHT }; // window width and height in SCREEN COORDINATES
    std::pair<int, int> m_framebuffer_size = { 0, 0 }; // window width and height in PIXELS, first set in start(), then callback

    bool m_fullscreen = false; // if custom fullscreen or not

    double m_start_time = 0.0; // time when frame started
};

int main()
{
    Layer layer; // setup code
    {
        int result = layer.start();
        if (result) return result;
    }

    // for window

        // shaders / programs
    unsigned int shaderProgram = compile_shader_program("vertexShader.glsl", "fragmentShader.glsl", "First Shader");
    unsigned int particleProgram = compile_shader_program("particleVertexShader.glsl", "particleFragmentShader.glsl", "Particle Shader");

    // vertex array object
    // triangle drawing stuff
    unsigned int VAO, VBO;
    {
        float vertices[] = {
            -0.5f, 0.25f, 0.0f,
             0.5f, 0.25f, 0.0f,
             0.0f,  1.0f, 0.0f
        };

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }

    unsigned int VAO_partic, VBO_partic, VBO_partic_instance;
    {
        glGenVertexArrays(1, &VAO_partic);
        glBindVertexArray(VAO_partic);

        float vertices[] =
          { 0.f, 0.f,
            1.0f, 0.0f,
            0.25f,-0.25f,
            0.0f, -1.f,
            -0.25f, -0.25f,
            -1.f, 0.f,
            -0.25f, 0.25f,
            0.f, 1.f,
            0.25f, 0.25f,
            1.0f, 0.0f,
        };

        glGenBuffers(1, &VBO_partic);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_partic);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        float offsets[32][2] = {{0.f, 0.f}};

        for (int i = 0; i < 32; ++i) {
            offsets[i][0] = -8.f + i*0.5;
            offsets[i][1] = i*0.5 - 8.f;
        }

        glGenBuffers(1, &VBO_partic_instance);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_partic_instance);
        glBufferData(GL_ARRAY_BUFFER, sizeof(offsets), offsets, GL_STATIC_DRAW);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        
        glVertexAttribDivisor(1, 1); // tell OpenGL this is an instanced vertex attribute.
    }

    // EBO
    unsigned int EBO_rect, VAO_rect, VBO_rect;
    {
        glGenVertexArrays(1, &VAO_rect);
        glGenBuffers(1, &VBO_rect);
        glGenBuffers(1, &EBO_rect);
        float vertices[] = {
             0.5f,  0.5f, 0.0f,  // top right
             0.5f, -0.5f, 0.0f,  // bottom right
            -0.5f, -0.5f, 0.0f,  // bottom left
            -0.5f,  0.5f, 0.0f   // top left 
        };
        unsigned int indices[] = {  // note that we start from 0!
            0, 1, 3,   // first triangle
            1, 2, 3    // second triangle
        };

        glBindVertexArray(VAO_rect);
        // 2. copy our vertices array in a vertex buffer for OpenGL to use
        glBindBuffer(GL_ARRAY_BUFFER, VBO_rect);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // 3. copy our index array in a element buffer for OpenGL to use
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_rect);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        // 4. then set the vertex attributes pointers
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    int time_uniform = glGetUniformLocation(shaderProgram, "time");
    int offset_uniform = glGetUniformLocation(shaderProgram, "offset");

    float x = 0.f;
    while (layer.loop_continue())
    {
        if (layer.key_state(GLFW_KEY_D).pressed) {
            x += 0.05f;
        }
        if (layer.key_state(GLFW_KEY_A).pressed) {
            x -= 0.05f;
        }
        if (layer.mouse_btn_state(GLFW_MOUSE_BUTTON_LEFT).pressed) {
            x += 0.05f;
        }
        x += layer.mouse_scroll().second/10.f;

        // drawing
        {
            

            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glUseProgram(shaderProgram); // uniforms
            glUniform1f(time_uniform, (float)glfwGetTime());
            glUniform2f(offset_uniform, x, 0.4 * sin(1.5 * glfwGetTime()));

            // EBO rectangle
            glUseProgram(shaderProgram);
            glBindVertexArray(VAO_rect);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            // particles
            glUseProgram(particleProgram);
            glBindVertexArray(VAO_partic);
            {
                float offsets[32][2] = { {0.f, 0.f} };

                for (int i = 0; i < 32; ++i) {
                    offsets[i][0] = -8.f + i * 0.5 + rand()%10;
                    offsets[i][1] = i * 0.5 - 8.f;
                }

                glBindBuffer(GL_ARRAY_BUFFER, VBO_partic_instance);
                glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(offsets), (float*)offsets);
            }

            glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 10, 32);
            
            // Triangle at mouse
            glUseProgram(shaderProgram);
            {
                auto mouse_pos = layer.mouse_pos_N();
                glUniform2f(offset_uniform, mouse_pos.first, mouse_pos.second);
                glUniform1f(time_uniform, glfwGetTime() * 1.5);
            }
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);

        }

        // code
        layer.end_of_loop();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    layer.clean_up();
}