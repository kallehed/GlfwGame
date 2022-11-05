#include "TextRenderer.h"
#include "Layer.h"

#include <iostream>

TextRenderer::TextRenderer()
{
    if (FT_Init_FreeType(&m_ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        std::cin.get();
    }

    if (FT_New_Face(m_ft, "fonts/Raleway.ttf", 0, &m_face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        std::cin.get();
    }
    FT_Set_Pixel_Sizes(m_face, 0, 48);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

    for (unsigned char c = 0; c < TOTAL_CHARACTERS; ++c)
    {
        // load character glyph 
        if (FT_Load_Char(m_face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            m_face->glyph->bitmap.width,
            m_face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            m_face->glyph->bitmap.buffer
        );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        Character character = {
            texture,
            std::make_pair(m_face->glyph->bitmap.width, m_face->glyph->bitmap.rows),
            std::make_pair(m_face->glyph->bitmap_left, m_face->glyph->bitmap_top),
            m_face->glyph->advance.x
        };
        m_characters[c] = character;
    }
    FT_Done_Face(m_face);
    FT_Done_FreeType(m_ft);

    m_program = Layer::compile_shader_program("textVertex.glsl", "textFragment.glsl", "Text Shader");

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);
    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    m_u_text_color = glGetUniformLocation(m_program, "u_text_color");

}

void TextRenderer::render_text(const char* text, float x, float y, float scale, std::array<float, 3> color)
{
    // activate corresponding render state
    glUseProgram(m_program);
    glUniform3f(m_u_text_color, color[0], color[1], color[2]);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(m_VAO);

    // iterate through all characters
    for (int i = 0; text[i] != '\0'; i++)
    {
        Character& ch = m_characters[text[i]];

        float xpos = x + ch.m_bearing.first * scale;
        float ypos = y - (ch.m_size.second - ch.m_bearing.second) * scale;

        float w = ch.m_size.first * scale;
        float h = ch.m_size.second * scale;
        // update VBO for each character
        //xpos = 0.f;
        //ypos = 0.f;
        /*const float div = 1000.f;
        xpos /= div;
        ypos /= div;
        w /= div;
        h /= div;*/
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.m_textureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.m_advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }
}