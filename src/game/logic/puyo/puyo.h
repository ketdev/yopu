#pragma once
#include "../../entity.h"

/*TEMP*/ #include "../../puyo.hpp"

namespace puyo {

    // Number of smaller units within a grid cells, 
    // independent of rendering proportions
    static constexpr int DROP_RES = 0x200000;

    // -- Components --

    enum class Color : uint8_t {
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

    struct GridIndex {
        int x, y;
        int drop = 0; // up to DROP_RES
    };

    struct Parent {
        entity player;  // Associated Player entity
    };

}
