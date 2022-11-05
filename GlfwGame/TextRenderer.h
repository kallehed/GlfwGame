#pragma once

#include <utility>
#include <array>

#include "ft2build.h"
#include "freetype/freetype.h"

struct Character {
	unsigned int m_textureID;  // ID handle of the glyph texture
	std::pair<int, int>   m_size;       // Size of glyph
	std::pair<int, int>   m_bearing;    // Offset from baseline to left/top of glyph
	int m_advance;    // Offset to advance to next glyph
};

class TextRenderer
{
public:
	TextRenderer();

	void render_text(const char* text, float x, float y, float scale, std::array<float, 3> color);

private:
	FT_Library m_ft;
	FT_Face m_face;

	static constexpr int TOTAL_CHARACTERS = 128;
	std::array<Character, TOTAL_CHARACTERS> m_characters;

	unsigned int m_program;
	unsigned int m_VAO, m_VBO;
	int m_u_text_color;
};

