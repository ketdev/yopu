#pragma once

#include "../graphics/vertex_array.h"
#include "../graphics/vertex_buffer.h"
#include "../graphics/shader.h"
#include "../graphics/texture.h"

#include <memory>

struct Rect {
	float x, y, w, h;
};

class SpriteRender {
public:
	SpriteRender();
	
	void update(int width, int height);
	void draw(const Texture& texture, const Rect& src, const Rect& dst, const glm::vec3& tint = { 1, 1, 1 });

private:
	glm::mat4 _projection;
	ShaderProgram _program;
	VertexBuffer _vbo;
	VertexArray _vao;
};