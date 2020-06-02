#pragma once
#include <vector>
#include "../entity.h"
#include "../puyo/puyo.h"

namespace player {

    // Constants
    static constexpr int groupPopSize = 4;

    // -- Tags --

    // Freefalling stage, waiting for all falling puyos to settle
    struct Freefalling {};

    // -- Components --

    struct Score {
        int sum = 0;
    };

    struct Chain {
        int length = 0;
    };

    // -- Systems --

    // RESOLVE SYSTEM [Player, Board, Chain]
    //  Calculate chains: Identify puyo color groups, pop them if any (enter freefall again)
    //  Sets board as Idle after all chains are resolved
    //  Adds Gravity to falling puyos after popping
    //  Adds Animation: Pop, Sending Chain
    //  Plays Sound: Chain [1-7] & Nuisance [S,M,L]
    void resolve(registry& reg);
}
