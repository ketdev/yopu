#pragma once
#include <stdint.h>
#include <vector>
#include <entt\entt.hpp>

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


    // -- Components --

    struct Parent {
        entt::entity entity;
    };
    struct Color {
        Type type;
    };
    struct GridIndex {
        int x, y;
    };
    struct ControlAxis {
        uint8_t animationCounter = 0;    // blinking animation
        uint8_t rotationCounter = 0;     // for double tap
        uint8_t midHeightCrossings = 0;  // prevent upward push more than 8 times

        entt::entity slave; // can have one slave only
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

    // ------------------------

    struct Acceleration {
        float acc;
    };


}  // namespace puyo
