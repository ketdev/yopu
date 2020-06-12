#include "vertex_buffer.h"
#include "../utils/xgl.h"

inline static GLuint createBuffer() {
	GLuint id;
	glGenBuffers(1, &id);
	return id;
}

VertexBuffer::VertexBuffer() : id(createBuffer()), dimension(0) {}

VertexBuffer::~VertexBuffer() { glDeleteBuffers(1, &id); }

void VertexBuffer::set(const void* data, uint32_t dimension, uint32_t count) {
	*const_cast<uint32_t*>(&this->dimension) = dimension;
	glBindBuffer(GL_ARRAY_BUFFER, id);
	glBufferData(GL_ARRAY_BUFFER, count * dimension * sizeof(float), data, GL_STATIC_DRAW);
}
