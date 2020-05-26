#pragma once
#include "../entity.h"

namespace puyo {

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
        int drop = 0;
    };

    struct Parent {
        entity entity;
    };

}
