#pragma once
#include <stdint.h>
#include <vector>
#include <entt\entt.hpp>
#include <SDL2\SDL_rect.h>

namespace puyo {

    // -- Typedefs --

    enum class Type : uint8_t {
        Red,
        Green,
        Blue,
        Yellow,
        Violet,

        Garbage,
        // Hard: 'Hard', // two to clear (becomes grey after one)
        // Iron: 'Iron', // need a 6 puyo clear to clear
        // Point: 'Point', // adds power to chain
    };


    // -- Update Components --

    struct Parent {
        entt::entity entity;
    };
    struct Color {
        Type type;
    };
    struct GridIndex {
        int x, y;
        int drop = 0;
    };
    struct ControlAxis {
        bool locked = false;            // once the player should not have control over it
        bool shift = false;             // if already performing a lateral move
        uint8_t rotationCounter = 0;    // for double tap
        uint8_t graceCounter = 0;       // frames before locking after pair is blocked
        uint8_t pushupCounter = 0;      // limit the number of upward pushes

        entt::entity slave; // can have one slave only
    };

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
