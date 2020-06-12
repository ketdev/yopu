#include "vertex_array.h"
#include "../utils/xgl.h"

inline static GLuint createVertexArray() {
	GLuint id;
	glGenVertexArrays(1, &id);
	return id;
}

VertexArray::VertexArray() : id(createVertexArray()) {}
VertexArray::~VertexArray() { glDeleteVertexArrays(1, &id); }


void VertexArray::setAttrib(uint32_t index,
	const VertexBuffer& buffer, bool normalized, size_t stride, const void* offset) {
	glBindVertexArray(id);
	glEnableVertexArrayAttrib(id, index);
	glBindBuffer(GL_ARRAY_BUFFER, buffer.id);
	glVertexAttribPointer(
		index, buffer.dimension, GL_FLOAT,
		normalized ? GL_TRUE : GL_FALSE,
		stride, offset
	);
}

void VertexArray::drawTriangles(uint32_t first, uint32_t count) {
	glBindVertexArray(id);
	glDrawArrays(GL_TRIANGLES, first, count);
}
