#pragma once
#include <stdint.h>


struct VertexBuffer {
	const uint32_t id;
	volatile const uint32_t dimension;

	VertexBuffer();
	~VertexBuffer();

	void set(const void* data, uint32_t dimension, uint32_t count);
};
