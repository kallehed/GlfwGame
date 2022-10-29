#pragma once

#include <utility>
#include <array>
#include <bitset>

class Layer;

class Life
{
public:
	Life();
	void logic(Layer& layer);
	void draw(Layer& layer);

private:
	void randomize(); // set matrix to random bool values
	void reset_to_0(); // set matrix to false for all values
	void next_generation(); // transform 

	static constexpr int m_SIZE = 2000; // how many cells in each direction
	static constexpr int m_TOTAL_CELLS = m_SIZE * m_SIZE;
	bool m_paused = true;
	float m_quad_length = 2.f/m_SIZE;
	std::pair<float, float> m_position = { -1.f, -1.f }; // offset viewing position, X AND Y
	std::array<std::bitset<m_TOTAL_CELLS>, 2> m_buffers;
	//std::array<std::array<bool,m_TOTAL_CELLS>, 2> m_buffers;
	int m_buf_nr = 0; // which buffer is currently active

	// opengl stuff
	unsigned int m_program = 0;
	unsigned int m_VAO, m_VBO, m_colors_VBO, m_EBO;

	// uniform locations
	int m_u_offset;
	int m_u_quad_length;
};