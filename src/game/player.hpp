#pragma once
#include <stdint.h>
#include <entt\entt.hpp>

#include "player\board.h"
#include "player\spawner.h"

/*
    Player states:
        - (Puyo) Control (wait until locked)
        - (Board) Resolve chains (wait until idle)
            - Freefall
            - Pop groups
            - Score calculation
            - [repeat]

        - Victory (external set)
        - Game Over (internal set)

    Systems:

        - [CONTROLLER] on Controlled Puyo:
            [Move] on Controlled Puyo & Move Input
                + Animation: Translate
            [Rotate] on Controlled Puyo & Rotation Input
                + Animation: Rotate
                + Animation: Translate
            [Soft Drop] on Controlled Puyo [optional Soft Drop Input]
                with Grace Period
                + Animation: Bouncing
            [Pair split]
                Wait for end of animations first
                + Falling Puyo

        - [FREEFALL] on Falling Puyo:
            Gravity initialization
                * Pair split takes 2 frames to complete if the main puyo should free-fall,
                * or 3 frames if it is the slave puyo. Technically, from the frame the pair
                * is detected as being blocked, a single frame passes without anything happening,
                * then the main puyo may begin its fall on the second frame after the blocking
                * event. The slave puyo may not begin its fall before the third frame.
            Freefall
            * When the gravity routine detects that the puyo has reached the floor, execution
            yields once again, calling the placement routine on the very next frame after
            free-fall. Both callbacks mark their respective object for cleanup and call the routine
            which will place the puyo on the virtual board representation in RAM.

            + Board settled event

        - [POP] on Settled Board

    Player Data:
        - Input gamepad
        - Board (?)

        - Score sum
        - score for current chaining step
        - score multiplier for current chaining step (10 * puyos cleared * variable factor, capped to 65535)
        - score multiplier for current chaining step (chain power + color bonus + group bonus, capped to 999)
        - current chain length
        - number of puyo groups to be cleared (for group bonus)
        - pending nuisance count while opponent is chaining (to fall on the structure's player)
        - pending nuisance count after opponent is done chaining (to fall on the structure's player)
        - callback pass counter (countdown: 0=call only once before advancing)

*/

namespace player {

	// -- Typedefs --

	enum InputKey {
		Left = 0,
		Right, 
		Up,
		Down, 
		RotateLeft,
		RotateRight,

		_Count
	};

	// -- Components --

	struct Input { 
		int controllerIndex = 0;
		struct Key {
			bool next, curr, trigger;
			int counter;
		} keys[InputKey::_Count];
	};

}  // namespace player
