#include "freefall.h"
#include "puyo.h"
#include "animate.h"
#include "../player/board.h"
#include "../player/resolve.h"
#include "../media/sound.h"
#include <iostream>

void puyo::freefall(registry& reg) {
    auto playerView = reg.view<player::Board, player::Freefalling>();
    for (auto& player : playerView) {
        auto& board = playerView.get<player::Board>(player);

        // Drop hanging puyos
        for (auto x = 0; x < player::Board::columns; x++) {
            int drop = 0;
            for (auto y = player::Board::rows - 1; y >= 0; y--) {
                if (!board.isBlocked({ x, y })) {
                    drop++;
                }
                else if (drop > 0 && board.isBlocked({ x,y })) {
                    std::cout << "Hang: (" << x << ", " << y << ") Drop: " << drop << std::endl;

                    // set at target
                    auto puyo = board.getCell({ x, y });
                    board.setCell({ x, y }, noentity);
                    board.setCell({ x, y + drop }, puyo);
                    reg.emplace_or_replace<puyo::GridIndex>(puyo, x, y + drop, DROP_RES);

                    // Add gravity animation
                    reg.emplace<puyo::GravityAnimation>(puyo, drop * TILE_SIZE, 1.0, 8.0, 0.1875);
                }

            }
        }

        // Wait while puyos are falling
        bool isFalling = false;
        auto gravityView = reg.view<puyo::Parent, puyo::GravityAnimation>();
        for (auto& puyo : gravityView) {
            auto& parent = gravityView.get<puyo::Parent>(puyo).player;
            if (parent == player) isFalling = true;
        }

        // Exit freefall stage
        if (!isFalling) {
            reg.remove<player::Freefalling>(player);
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
