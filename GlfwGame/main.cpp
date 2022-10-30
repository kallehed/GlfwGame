#include "Layer.h"
#include "Life.h"

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>
#include <array>

#include "stb_image.h"

int main()
{
    Layer layer; // setup code
    {
        int result = layer.start();
        if (result) return result;
    }

    // for window

        // shaders / programs
    unsigned int shaderProgram = Layer::compile_shader_program("vertexShader.glsl", "fragmentShader.glsl", "First Shader");
    unsigned int particleProgram = Layer::compile_shader_program("particleVertexShader.glsl", "particleFragmentShader.glsl", "Particle Shader");
    unsigned int imageProgram = Layer::compile_shader_program("imageVertex.glsl", "imageFragment.glsl", "Image Shader");
    
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

    // image
    unsigned int VAO_image, VBO_image, EBO_image, texture;
    {
        glGenVertexArrays(1, &VAO_image);
        glGenBuffers(1, &VBO_image);
        glGenBuffers(1, &EBO_image);
        glGenBuffers(1, &VBO_image);

        float vertices[] =
        {
            //x   y   tex x   y
             0.0, 0.0,   0.0, 0.0, // bottom left
             1.0, 0.0,   1.0, 0.0, // bottom right
             1.0, 1.0,   1.0, 1.0, // top right
             0.0, 1.0,   0.0, 1.0 // top left
        };
        unsigned int indices[] =
        {
            0, 1, 2, // first triangle
            2, 3, 0// second triangle
        };

        glBindVertexArray(VAO_image);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_image);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_image);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        constexpr GLsizei stride = 4 * sizeof(float);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // texture
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // s = x
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // t = y    because: str = xyz

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // texture scaling
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int width, height, nrChannels;
        unsigned char* data = stbi_load("img.png", &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Failed to load texture" << std::endl;
        }
        stbi_image_free(data);
    }

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    int time_uniform = glGetUniformLocation(shaderProgram, "time");
    int offset_uniform = glGetUniformLocation(shaderProgram, "offset");

    Life life;
    float x = 0.f;

    // game of life
    while (layer.loop_continue())
    {
        life.logic(layer);

        if (layer.mouse_btn_state(GLFW_MOUSE_BUTTON_LEFT).pressed) {
            x += 0.05f;
        }
        
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

            // image / texture
            glUseProgram(imageProgram);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);
            glBindVertexArray(VAO_image);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            life.draw(layer);

        }
        // code
        layer.end_of_loop();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    layer.clean_up();
}