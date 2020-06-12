#pragma once

#include "../graphics/vertex_array.h"
#include "../graphics/vertex_buffer.h"
#include "../graphics/shader.h"

class Render {
public:
	Render();
	void draw();

private:
	ShaderProgram _program;
	VertexBuffer _vbo;
	VertexArray _vao;
};