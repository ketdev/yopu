#pragma once
#include <stdint.h>
#include <vector>
#include <entt\entt.hpp>
#include <SDL2\SDL_rect.h>

namespace puyo {

    const int TILE_SIZE = 32;


    // -- Render Components --

    struct RenderPosition {
        int x, y;
    };
    struct TranslateAnimation {
        int dx, dy;
        int frames;
    };
    struct RotateAnimation {
        int srcDx, srcDy;
        int dstDx, dstDy;
        int frames;
    };

    struct BlinkingAnimation {
        // 10 on, 10 off, repeat
        uint8_t counter = 0;
    };
    struct BounceAnimation {
        // Bounce: down - mid - up - mid - down - mid - up - mid (16 frames total?, 2 each)
        uint8_t counter = 0;
    };

}  // namespace puyo
