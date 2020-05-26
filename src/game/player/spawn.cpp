#include "spawner.h"
#include "board.h"
#include "../puyo/puyo.h"
#include <iostream>

// Updates shared pool and returns the puyo at a given spawn index
//      spawnPool: a shared spawn pool used for all players
//      index: puyo sequence index
//      colorCount: Number of playable colors (3-5)
static puyo::Type nextPuyo(player::Spawner& spawner) {
    // Create more in pool if needed
    while (spawner.poolIndex >= spawner.pool->size()) {
        // Get random puyo color
        auto type = static_cast<puyo::Type>((*spawner.randgen)() % spawner.colorCount);
        spawner.pool->push_back(type);
    }
    return (*spawner.pool)[spawner.poolIndex];
}

static entity makePuyo(registry& reg, puyo::Type type, entity player, puyo::GridIndex pos) {
    auto puyo = reg.create();
    reg.emplace<puyo::Parent>(puyo, player);       // Associated player entity
    reg.emplace<puyo::Color>(puyo, type);
    reg.emplace<puyo::GridIndex>(puyo, pos.x, pos.y, 0);

    // ---
    // TODO: render position based on board, shift, etc...
    reg.emplace<puyo::RenderPosition>(puyo, pos.x * TILE_SIZE, pos.y * TILE_SIZE);
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
        
        // first is axis, second slave
        entt::entity axis = noentity;
        for (auto pos : spawner.location) {

            // Game Over if one spawner cell is not empty
            if (noentity != board.grid[pos.y][pos.x]) {
                reg.emplace<player::GameOver>(player);
                return;
            }

            // Create puyo on spawner
            auto type = nextPuyo(spawner);
            auto puyo = makePuyo(reg, type, player, pos);

            // Set first spawned puyo as controlled
            if (axis == noentity) {
                reg.emplace<puyo::ControlAxis>(puyo);
                reg.emplace<puyo::BlinkingAnimation>(puyo);
                axis = puyo;
            }
            // Set slave parent to axis and axis slave to puyo
            else {
                reg.get<puyo::Parent>(puyo).entity = axis;
                reg.get<puyo::ControlAxis>(axis).slave = puyo;
            }

            std::cout << "Spawn Index: " << spawner.poolIndex << " Type: " << static_cast<int>(type) << std::endl;
        }

        reg.remove<player::Idle>(player);
    }
}
