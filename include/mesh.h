#pragma once

#include "Nexus/Window/GLFWWindow.h"
#include "shader.h"
#include "utils.h"
#include <string>
#include <vector>

struct Vertex {
	Point position;
	Direction normal;
	TextureCoord texCoord;
	Color color;
};

class Mesh {
public:
	Mesh(std::string filepath);
	Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);
	
	void render(Shader &shader, const glm::mat4 &model, const glm::mat4 &view, const glm::mat4 &projection);
private:
	void initOpenGLBuffers();
private:
	std::string m_name;
	std::vector<Vertex> m_vertices;
	std::vector<GLuint> m_indices;

	GLuint m_vao, m_vbo, m_ebo;

};