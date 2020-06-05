#pragma once
#include "../entity.h"
#include "../../engine/sdl_utils.hpp"

namespace media {

    // -- Components --

    enum class Texture {
        Background = 0,

        _Count,
    };

    struct Position {
        int x, y;
    };

    // -- Systems --

    void spriteRender(registry& reg, SDL::Renderer& renderer);

}