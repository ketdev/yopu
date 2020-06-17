#pragma once
#include <stdint.h>
#include <string>

struct Texture {
	const uint32_t id;

	Texture();
	~Texture();

	void load(const std::string& path);
	void use();
};

