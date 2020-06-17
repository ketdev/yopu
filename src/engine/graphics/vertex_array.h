#pragma once
#include <stdint.h>
#include "vertex_buffer.h"

struct VertexArray {
	const uint32_t id;

	VertexArray();
	~VertexArray();

	void setAttrib(uint32_t index, const VertexBuffer& buffer,
		bool normalized, size_t stride, const void* offset);

	void drawTriangles(uint32_t first, uint32_t count);
	void drawTriangleStrips(uint32_t first, uint32_t count);
};
