#pragma once

#include "../graphics/vertex_array.h"
#include "../graphics/vertex_buffer.h"
#include "../graphics/shader.h"
#include "../graphics/texture.h"

class Render {
public:
	Render();
	void draw();

private:
	ShaderProgram _program;
	VertexBuffer _vbo;
	VertexArray _vao;
	Texture _texture;
};