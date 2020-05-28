#include "freefall.h"
#include "puyo.h"
#include "../player/board.h"
#include "../player/chain.h"
#include "../media/sound.h"
#include <iostream>

void puyo::freefall(registry& reg) {
    for (auto& player : reg.view<player::Freefalling>()) {
        auto view = reg.view<puyo::GridIndex, puyo::RenderPosition, puyo::Parent, puyo::Gravity>();

        bool falling = false;
        for (auto& puyo : view) {

            // Filter to relevant player in freefalling stage
            if (view.get<puyo::Parent>(puyo).player != player) continue;
            falling = true;

            // Get main and slave structures
            auto& index = view.get<puyo::GridIndex>(puyo);
            auto& gravity = view.get<puyo::Gravity>(puyo);
            auto& renderPos = view.get<puyo::RenderPosition>(puyo);

            // Wait a few frames before starting freefall
            if (gravity.fallDelay > 0) {
                gravity.fallDelay--;
                continue;
            }

            // Get board from associated player
            if (!reg.has<player::Board>(player)) {
                continue;
            }
            auto& board = reg.get<player::Board>(player);

            // Apply freefall displacement
            index.drop += gravity.speed;

            // Accelerate and cap speed
            gravity.speed = std::min(gravity.speed + gravity.acceleration, gravity.terminalSpeed);

            // Check if passed a board cell
            if (index.drop >= puyo::DROP_RES) {
                // If the new cell is blocked, set to the bottom of the last free cell
                if (board.isBlocked({ index.x, index.y + 1 })) {
                    index.drop = puyo::DROP_RES;

                    // Play bouncing animation
                    reg.emplace<puyo::BounceAnimation>(puyo);

                    // Play placement sound effect 
                    media::play(reg, media::SoundEffect::Drop);

                    // Finish freefall here
                    reg.remove<puyo::Gravity>(puyo);
                    board.setCell(index, puyo);
                }
                else {
                    // advance cell
                    index.drop -= puyo::DROP_RES;
                    index.y++;
                }
            }

            // Update render coordinate
            /*TEMP*/ renderPos.y = index.y * puyo::TILE_SIZE + index.drop * puyo::TILE_SIZE / puyo::DROP_RES;
        }

        // When placed last falling puyo, start chain resolving
        if (!falling) {
            reg.remove<player::Freefalling>(player);
            reg.emplace<player::Chain>(player);
        }
    }

    /*
    Cases: Free fall speed differs for:
        puyos controlled by the player, after splitting a pair in two;
        single ojama puyos;
        bulk ojama puyos, depending on their respective column;
        when a chain disappears, puyos above the hole fill the gap by free-falling.


    Gravity acceleration

    Placement
        * When the gravity routine detects that the puyo has reached the floor, execution
        yields once again, calling the placement routine on the very next frame after
        free-fall. Both callbacks mark their respective object for cleanup and call the routine
        which will place the puyo on the virtual board representation in RAM.

    + Board settled event

    */
}