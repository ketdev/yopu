#pragma once
#include <set>
#include "../entity.h"
#include "../puyo/puyo.h"

namespace player {

    // -- Tags --

    // Freefalling stage, waiting for all falling puyos to settle
    struct Freefalling {};

    // -- Components --

    struct Score {
        int sum = 0;

        // TODO: add to other player, not self
        int chainingGarbage = 0; // pending during chaining process
        int garbage = 0; // ready to be dropped
        double garbageLeftover = 0;
    };

    struct Chain {
        static constexpr int popSize = 4;
        static constexpr int nuisanceCost = 70;

        int length = 0;
        int sizePower = 0;
        int chainPower = 0;

        int scoreSum = 0;
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
