#pragma once
#include <stdint.h>

namespace object {

	struct Puyo2 {
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

        Puyo2::Type type;
        int x, y, ysub;
    };

}