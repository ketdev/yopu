#pragma once
#include <random>
#include "../entity.h"
#include "../puyo/puyo.h"

namespace player {

    // -- Tags --

    // When we should spawn more puyos
    struct Idle {};

    // When player lost
    struct GameOver {};

    // -- Components --

    struct Spawner {
        // Puyo spawn locations
        static constexpr puyo::GridIndex mainSpawn = { 3, 1 };
        static constexpr puyo::GridIndex slaveSpawn = { 3, 0 };

        // Number of playable colors (3-5)
        int colorCount = 5;

        // Shared puyo pool
        std::shared_ptr<std::mt19937> randgen;
        std::shared_ptr<std::vector<puyo::Color>> pool;

        // A running index in the shared spawn pool
        uint32_t poolIndex = {};
    };

    // -- Systems --

    // SPAWN SYSTEM [Board, Spawner, Idle]
    //  Spawn and activate puyo control
    //  Adds Controlled Puyo and slave Puyo
    //  Adds Animation: Blinking
    void spawn(registry& reg);

    // TODO: optimization, shared pool struct with all subscribers, remove already processed index, save memory
    // TODO: spawn garbage puyos as well
}

