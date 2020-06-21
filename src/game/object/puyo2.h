#pragma once
#include <stdint.h>

namespace puyo {

    /*
	struct Puyo2 {
        // Number of smaller units within a grid cells, 
        // independent of rendering proportions
        static constexpr int SubRes = 0x200000;

        enum class Type : uint8_t {
            None,

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
        int x, y, ysub = 0;
    };
    */

    struct Control {
        int dropSpeed = DROP_RES / 16; // 16 frames needed to cross a cell

        static constexpr int softDropSpeed = DROP_RES / 2;   // 2 frames needed to cross a cell when pushing down
        static constexpr int gracePeriod = 32;               // frames the player can still move the puyo after hitting a floor
        static constexpr int maxPushups = 8;                 // maximal number of pushing up caused by rotations
        static constexpr int splitFallDelay = 2;

        bool locked = false;            // once the player should not have control over it

        bool shift = false;             // if already performing a lateral move
        uint8_t rotationCounter = 0;    // for double tap
        uint8_t graceCounter = 0;       // frames before locking after pair is blocked
        uint8_t pushupCounter = 0;      // limit the number of upward pushes

        int splitCounter = 0;           // Delay frames between splitting to freefall

        object slave;   // can have one slave only
    };


}