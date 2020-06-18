#include "spawn.h"
#include "board.h"
#include "resolve.h"
#include "freefall.h"
#include "../puyo/puyo.h"
#include "../puyo/control.h"
#include "../../media/sprite.h"
#include <algorithm>
#include <iostream>

// Return a random number from 0 to max
static int randomInt(player::Spawner& spawner, int max) {
    return ((*spawner.randgen)() % max);
}

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
    puyo::Color color = (*spawner.pool)[spawner.poolIndex];
    spawner.poolIndex++;
    spawner.poolIndex %= 128;
    return color;
}

static entity makePuyo(registry& reg, puyo::Color type, puyo::GridIndex pos, entity player) {
    auto puyo = reg.create();
    reg.emplace<puyo::Color>(puyo, type);
    reg.emplace<puyo::GridIndex>(puyo, pos.x, pos.y, 0);
    reg.emplace<puyo::Parent>(puyo, player);

    // ---
    constexpr int OFFSET_X = 119;
    constexpr int OFFSET_Y = 263;

    // TODO: render position based on board, shift, etc...
    reg.emplace<puyo::RenderPosition>(puyo, 
        OFFSET_X + pos.x * puyo::TILE_SIZE,
        OFFSET_Y + pos.y * puyo::TILE_SIZE + pos.drop * puyo::TILE_SIZE / puyo::DROP_RES);
    

    //reg.emplace<media::Texture>(puyo, media::Texture::Puyo);
    //reg.emplace<media::Position>(puyo,
    //    OFFSET_X + pos.x * puyo::TILE_SIZE,
    //    OFFSET_Y + pos.y * puyo::TILE_SIZE + pos.drop * puyo::TILE_SIZE / puyo::DROP_RES);

    reg.emplace<media::Sprite>(puyo, media::Texture::Puyo,
        glm::ivec2{ OFFSET_X + pos.x * 120,
                    OFFSET_Y + pos.y * 120 + pos.drop * 120 / puyo::DROP_RES },
        glm::ivec2{ 120, 120 },
        glm::dvec4{ 0, 0, 1, 1 }, 1);
    // ---

    return puyo;
}

// Spawns puyos and garbages to the player board, 
// if spawner slots are full, sets the Game Over tag on the player
// + Animation: Blinking
void player::spawn(registry& reg) {
    auto view = reg.view<player::Board, player::Score, player::Spawner, player::Idle>();
    for (auto& player : view) {
        auto& board = view.get<player::Board>(player);
        auto& score = view.get<player::Score>(player);
        auto& spawner = view.get<player::Spawner>(player);
        
        // Game Over if one spawner cell is not empty
        if (board.isBlocked(spawner.mainSpawn) || board.isBlocked(spawner.slaveSpawn)) {
            reg.emplace_or_replace<player::GameOver>(player);
            std::cout << "Game Over" << std::endl;
            return;
        }

        // Drop garbage if didn't pop last round
        if (!score.popLastTurn && score.garbage) {

            std::cout << "Dropping garbage: " << score.garbage << std::endl;

            // Spawn garbage rows
            if (score.garbage > player::Board::columns) {
                int chunk = std::min(player::Score::maxGarbageRows * player::Board::columns, score.garbage);
                score.garbage -= chunk;

                // TODO
            }
            // Spawn individual garbage puyos
            else {
                bool spawnCol[player::Board::columns] = {};
                for (auto i = 0; i < score.garbage; i++) {
                    // Get free column to spawn garbage
                    int column = randomInt(spawner, player::Board::columns - i);
                    while (spawnCol[column]) {
                        column = ((column + 1) % player::Board::columns);
                    }
                    spawnCol[column] = true;

                    // Create garbage puyos on top row
                    puyo::GridIndex pos = { column, 0, puyo::DROP_RES };
                    auto garbage = makePuyo(reg, puyo::Color::Garbage, pos, player);
                    board.setCell(pos, garbage);
                }
                score.garbage = 0;
            }         

            // Skip control phase
            reg.emplace<player::Freefalling>(player);
            reg.emplace<player::Chain>(player);
        }
        else {
            // Reset for new spawn turn
            score.popLastTurn = false;

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
        }

        reg.remove<player::Idle>(player);
    }
}
