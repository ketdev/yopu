#include "spawner.h"
#include "board.h"
#include "../puyo/puyo.h"
#include "../puyo/control.h"
#include <iostream>

// Updates shared pool and returns the puyo at a given spawn index
//      spawnPool: a shared spawn pool used for all players
//      index: puyo sequence index
//      colorCount: Number of playable colors (3-5)
static puyo::Color nextColor(player::Spawner& spawner) {
    // Create more in pool if needed
    while (spawner.poolIndex >= spawner.pool->size()) {
        // Get random puyo color
        auto type = static_cast<puyo::Color>((*spawner.randgen)() % spawner.colorCount);
        spawner.pool->push_back(type);
    }
    return (*spawner.pool)[spawner.poolIndex];
}

static entity makePuyo(registry& reg, puyo::Color type, puyo::GridIndex pos, entity player) {
    auto puyo = reg.create();
    reg.emplace<puyo::Color>(puyo, type);
    reg.emplace<puyo::GridIndex>(puyo, pos.x, pos.y, 0);
    reg.emplace<puyo::Parent>(puyo, player);

    // ---
    // TODO: render position based on board, shift, etc...
    reg.emplace<puyo::RenderPosition>(puyo, pos.x * puyo::TILE_SIZE, pos.y * puyo::TILE_SIZE);
    // ---

    return puyo;
}

// Spawns puyos and garbages to the player board, 
// if spawner slots are full, sets the Game Over tag on the player
//      reg: our entity registry
//      spawnPool: a shared spawn pool used for all players
void player::spawn(registry& reg) {
    auto view = reg.view<player::Board, player::Spawner, player::Idle>();
    for (auto& player : view) {
        auto& board = view.get<player::Board>(player);
        auto& spawner = view.get<player::Spawner>(player);
        
        // Game Over if one spawner cell is not empty
        if (board.isBlocked(spawner.mainSpawn) || board.isBlocked(spawner.slaveSpawn)) {
            reg.emplace<player::GameOver>(player);
            return;
        }

        // Create puyo on spawners
        auto mainColor = nextColor(spawner);
        auto slaveColor = nextColor(spawner);
        auto mainPuyo = makePuyo(reg, mainColor, spawner.mainSpawn, player);
        auto slavePuyo = makePuyo(reg, slaveColor, spawner.slaveSpawn, player);

        // Set main puyo as controlled, with a reference to slave puyo
        reg.emplace<puyo::Control>(mainPuyo).slave = slavePuyo;
        reg.emplace<puyo::BlinkingAnimation>(mainPuyo);

        std::cout << "Spawn Main: " << spawner.poolIndex << " Type: " << static_cast<int>(mainColor) << std::endl;
        std::cout << "Spawn Slave: " << spawner.poolIndex << " Type: " << static_cast<int>(slaveColor) << std::endl;

        reg.remove<player::Idle>(player);
    }
}
