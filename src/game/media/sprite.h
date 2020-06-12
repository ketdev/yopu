#pragma once
#include "../entity.h"
#include "../../engine/utils/sdl_utils.h"
#include <glm/vec4.hpp>
#include <glm/ext/vector_int2.hpp>

namespace media {

    enum class Texture {
        Background = 0,
        Puyo,

        _Count,
    };

    // -- Components --

    struct Sprite {
        Texture texture;
        glm::ivec2 pos;
        glm::ivec2 size;
        glm::dvec4 uv = { 0, 0, 1, 1 };
        int layer = 0;
    };

    // -- Systems --

    void spriteRender(registry& reg, SDL::Renderer& renderer);

}