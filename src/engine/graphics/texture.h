#pragma once
#include <stdint.h>
#include <string>

struct Texture {
	const uint32_t id;
	const uint32_t width, height;

	Texture();
	~Texture();

	void load(const std::string& path);
	void use() const;
};

