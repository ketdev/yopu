#pragma once
#include <game/object/object.h>
#include <game/object/player.h>

namespace play {

    // -- Constants --

	static constexpr int PlayerCount = 2;


    // -- Enumerations --

    enum class Phase {
        Settled,
        Control,
        Freefall,
        Chain,
        GameOverWin,
        GameOverLose
    };

    // -- Structures --

    struct Player {
        Phase phase;
    };

    // -- Game Logic State --

    struct State {
        Player players[PlayerCount];
    };

	void step(State& play, registry& reg);
}