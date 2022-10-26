#pragma once

#include <utility>
#include <array>

class Layer;

class Life
{
public:
	Life();

	void logic(Layer& layer);

	void draw(Layer& layer);
private:
	void randomize(); // set matrix to random bool values
	void next_generation(); // transform 

	static constexpr int m_SIZE = 50; // how many cells in each direction
	bool m_paused = true;
	float m_zoom = 2.f; // how big things should be
	std::pair<float, float> m_position = { -1.f, -1.f }; // offset viewing position, X AND Y
	std::array<std::array<bool, m_SIZE>, m_SIZE> m_matrix = { {false} };


	// opengl stuff
	unsigned int m_program = 0;
	unsigned int m_VAO, m_VBO, m_EBO;

	int m_u_color;
	int m_u_position;
	int m_u_scale;
};