#pragma once
#include "../entity.h"
#include "../puyo/puyo.h"
#include <random>

namespace player {

    // -- Tags --

    // When we should spawn more puyos
    struct Idle {};

    // When player lost
    struct GameOver {};

    // -- Components --

    struct Spawner {
        // Puyo spawn locations, first being axis
        static constexpr int count = 2;
        static constexpr puyo::GridIndex location[count] = {
            { 3, 1 }, // main spawn
            { 3, 0 },
        };

        // Number of playable colors (3-5)
        int colorCount = 5;

        // Shared puyo pool
        std::shared_ptr<std::mt19937> randgen;
        std::shared_ptr<std::vector<puyo::Type>> pool;

        // The index of the shared spawn pool
        uint32_t poolIndex = {};
    };

    // -- Systems --

    // SPAWN SYSTEM [Board, Spawner, Idle]
    //  Spawn and activate puyo control
    //  Adds Controlled Puyo and slave Puyo
    //  Adds Animation: Blinking
    void spawn(registry& reg);

    // TODO: spawn garbage puyos as well
}

