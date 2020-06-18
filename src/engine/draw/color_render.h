#pragma once

#include "../graphics/vertex_array.h"
#include "../graphics/vertex_buffer.h"
#include "../graphics/shader.h"
#include "../graphics/texture.h"

#include <memory>

class ColorRender {
public:
	ColorRender();
	
	void update(int width, int height);
	void draw(glm::vec2 pos, glm::vec2 size, glm::vec4 color);

private:
	glm::mat4 _projection;
	ShaderProgram _program;
	VertexBuffer _vbo;
	VertexArray _vao;
};