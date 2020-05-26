#pragma once
#include <stdint.h>
#include <vector>
#include <entt\entt.hpp>
#include <SDL2\SDL_rect.h>

#include "puyo\puyo.h"
#include "puyo\control.h"

namespace puyo {


    // -- Render Components --

    struct Sprite {
        //SDL::Texture& texure;
        SDL_Rect dst, src;
    };


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
        uint8_t counter = 0;
    };
    struct BounceAnimation {
        uint8_t counter = 0;
    };

    // ------------------------

    struct Acceleration {
        float acc;
    };


}  // namespace puyo
