#pragma once
#include <stdint.h>
#include "../../entity.h"
#include "puyo.h"

namespace puyo {

    // -- Components --

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

        entity slave;   // can have one slave only
    };

    // -- Systems --

    // CONTROL SYSTEM [Puyo, Control]
    //  Move, Rotate and soft drop a puyo
    //  Adds Falling Puyo pair after end of user control
    //  Adds Animation: Movement, Rotation, Bounce
    //  Plays Sound: Move, Rotate, Drop
    void control(registry& reg);

}
