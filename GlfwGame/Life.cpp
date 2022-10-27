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

    }
    m_u_color = glGetUniformLocation(m_program, "u_color");
    m_u_position = glGetUniformLocation(m_program, "u_position");
    m_u_scale = glGetUniformLocation(m_program, "u_scale");
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
    if (layer.key_state(GLFW_KEY_R).just_pressed) {
        randomize();
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

        const float scale = m_zoom / m_matrix.size();

        for (int i = 0; i < m_matrix.size(); ++i) {
            for (int j = 0; j < m_matrix.size(); ++j) {
                bool alive = m_matrix[i][j];

                glUniform1f(m_u_color, (float)alive);
                glUniform2f(m_u_position, m_zoom * (float)j / (float)m_matrix[0].size() + m_position.first, m_zoom * (float)i / (float)m_matrix.size() + m_position.second);
                glUniform1f(m_u_scale, m_zoom / m_matrix.size());
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            }
        }
    }
}

void Life::randomize()
{
    for (int i = 1; i < m_SIZE - 1; ++i) {
        for (int j = 1; j < m_SIZE - 1; ++j) {
            m_matrix[i][j] = ((rand() % 2) == 0);
        }
    }
}

void Life::next_generation()
{
    std::array<std::array<bool, m_SIZE>, m_SIZE> matrix_copy;
    for (int i = 0; i < m_SIZE; ++i) {
        for (int j = 0; j < m_SIZE; ++j) {
            matrix_copy[i][j] = m_matrix[i][j];
        }
    }
    // apply algorithm on all EXCEPT BORDERS
    for (int i = 1; i < m_SIZE - 1; ++i) {
        for (int j = 1; j < m_SIZE - 1; ++j) {
            auto& m = matrix_copy; // shorthand
            int neighbors = m[i-1][j-1]+ m[i-1][j] + m[i-1][j+1] +
                            m[i][j-1]        +       m[i][j+1] +
                            m[i+1][j-1] + m[i+1][j] + m[i+1][j+1];
            
            // if alive, alive if 2 or 3 neighbors, if dead, alive if 3 neighbors
            m_matrix[i][j] = matrix_copy[i][j] ? (2 <= neighbors && neighbors <= 3) : (neighbors == 3);
        }
    }
}
