#include "Life.h"
#include "Layer.h"

Life::Life() {
    // random start seed
    randomize();

    m_program = Layer::compile_shader_program("lifeVertex.glsl", "lifeFragment.glsl", "Life Shader");

    // game of life rect
    {
        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);

        float vertices[] = {
            0.f, 0.f,
            0.f, 1.f,
            1.f, 1.f,
            1.f, 0.f
        };

        unsigned int indices[] = {
            0, 1, 2,
            0, 2, 3
        };

        glGenBuffers(1, &m_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glGenBuffers(1, &m_EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        // data for colors 

        glGenBuffers(1, &m_colors_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_colors_VBO);
        std::array<float, m_TOTAL_CELLS> colors = {0.0f};
        
        glBufferData(GL_ARRAY_BUFFER, sizeof(colors), (float*)&colors, GL_DYNAMIC_DRAW); // lesson: GPU seems to handle any amount of data

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(float), (void*)0);
        glVertexAttribDivisor(1, 1);
    }
    m_u_offset = glGetUniformLocation(m_program, "u_offset");
    m_u_quad_length = glGetUniformLocation(m_program, "u_quad_length");
}

void Life::logic(Layer& layer)
{
    if (layer.key_state(GLFW_KEY_D).pressed) {
        m_position.first += 0.05f;
    }
    if (layer.key_state(GLFW_KEY_A).pressed) {
        m_position.first -= 0.05f;
    }
    if (layer.key_state(GLFW_KEY_W).pressed) {
        m_position.second += 0.05f;
    }
    if (layer.key_state(GLFW_KEY_S).pressed) {
        m_position.second -= 0.05f;
    }

    {
        float scroll = layer.mouse_scroll().second;
        std::pair<float, float> mouse_pos = layer.mouse_pos_N();
        if (scroll != 0.f) {
            m_zoom += scroll / 10.f;
        }
    }
    
    if (layer.key_state(GLFW_KEY_P).just_pressed) {
        m_paused = !m_paused;
    }
    if (layer.key_state(GLFW_KEY_R).pressed) {
        randomize();
    }
    if (layer.key_state(GLFW_KEY_T).just_pressed) {
        reset_to_0();
    }

    // more LIFEY logic
    if (m_paused) {
        if (layer.key_state(GLFW_KEY_SPACE).just_pressed) { // next generation
            next_generation();
        }
    }
    else {
        next_generation();
    }
}

void Life::draw(Layer& layer)
{
    // draw game of life
    {
        glUseProgram(m_program);
        glBindVertexArray(m_VAO);

        const float scale = m_zoom / m_SIZE;

        float colors[m_SIZE][m_SIZE];
        
        const float p_inc = m_zoom / m_SIZE;
        glUniform1f(m_u_quad_length, p_inc);
        glUniform2f(m_u_offset, m_position.first, m_position.second);

        for (int i = 0; i < m_SIZE; ++i) {
            for (int j = 0; j < m_SIZE; ++j) {
                bool alive = m_buffers[m_buf_nr][i * m_SIZE + j];
                colors[i][j] = (float)alive;
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, m_colors_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(colors), (float*)colors);

        //glUniform1fv(m_u_color, m_TOTAL_CELLS, (float*)u_colors);
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, m_TOTAL_CELLS);
    }
}

void Life::randomize()
{
    for (int i = 1; i < m_SIZE - 1; ++i) {
        for (int j = 1; j < m_SIZE - 1; ++j) {
            m_buffers[m_buf_nr][i * m_SIZE + j] = ((rand() % 2) == 0);
        }
    }
}

void Life::reset_to_0()
{
    for (int i = 1; i < m_SIZE - 1; ++i) {
        for (int j = 1; j < m_SIZE - 1; ++j) {
            m_buffers[m_buf_nr][i * m_SIZE + j] = false;
        }
    }
}

void Life::next_generation() // set (1 - m_buf_nr) to new buffer, then copy 
{
    const int old_buf = m_buf_nr;
    m_buf_nr = 1 - old_buf;
    const int new_buf = m_buf_nr;

    // apply algorithm on all EXCEPT BORDERS
    for (int i = 1; i < m_SIZE - 1; ++i) {
        for (int j = 1; j < m_SIZE - 1; ++j) {
            auto& m = m_buffers[old_buf]; // shorthand
            const int p_up = (i - 1) * m_SIZE + j, p_mid = (i) * m_SIZE + j, p_down = (i + 1) * m_SIZE + j;
            int neighbors = m[p_up - 1]   + m[p_up]   + m[p_up + 1] +
                            m[p_mid - 1]  +             m[p_mid + 1] +
                            m[p_down - 1] + m[p_down] + m[p_down + 1];
            
            // if alive, alive if 2 or 3 neighbors, if dead, alive if 3 neighbors
            m_buffers[new_buf][p_mid] = m[p_mid] ? (2 <= neighbors && neighbors <= 3) : (neighbors == 3);
        }
    }
}
