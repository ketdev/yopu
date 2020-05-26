#pragma once
#include <stdint.h>
#include "../entity.h"

namespace puyo {

    // -- Constants --

    const int TILE_SIZE = 32;

    // -- Components --

    struct ControlAxis {
        int dropSpeed = TILE_SIZE / 16; // 16 frames needed to cross a cell

        static constexpr int softDropSpeed = TILE_SIZE / 2;  // 2 frames needed to cross a cell when pushing down
        static constexpr int gracePeriod = 32;               // frames the player can still move the puyo after hitting a floor
        static constexpr int maxPushups = 8;                 // maximal number of pushing up caused by rotations

        bool locked = false;            // once the player should not have control over it

        bool shift = false;             // if already performing a lateral move
        uint8_t rotationCounter = 0;    // for double tap
        uint8_t graceCounter = 0;       // frames before locking after pair is blocked
        uint8_t pushupCounter = 0;      // limit the number of upward pushes

        entity slave; // can have one slave only
    };

    // -- Systems --

    // CONTROL SYSTEM [Puyo, Control]
    //  Move, Rotate and soft drop a puyo
    //  Adds Falling Puyo pair after end of user control
    //  Adds Animation: Movement, Rotation, Bounce
    //  Plays Sound: Move, Rotate, Drop
    void control(registry& reg);
}
