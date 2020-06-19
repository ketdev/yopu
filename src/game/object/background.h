#pragma once
#include "../../engine/graphics/texture.h"
#include "../../engine/loader.hpp"

struct Background {
	static constexpr char ASSET[] = "src/assets/texture/background.png";

	Texture texture;

	Background() {
		texture.load(ASSET);
	}
};