#pragma once
#include <random>
#include <game/object/object.h>

/*TEMP*/ #include <game/logic/puyo/puyo.h>

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


    // -- Tags --

    // When we should spawn more puyos
    struct Idle {};

    // Freefalling stage, waiting for all falling puyos to settle
    struct Freefalling {};

    // When player lost
    struct GameOver {};


	// -- Components --

	struct Input {
		static constexpr int buttonRepeatDelay = 8;
		static constexpr int buttonSubsequentDelay = 2;

		int controllerIndex = 0;
		struct Key {
			bool isDown, repeat;
			int counter;
		} keys[InputKey::_Count];

		// Triggers
		int dx = 0;
		int dr = 0;
		bool softDrop;
	};

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

    struct Score {
        static constexpr int maxGarbageRows = 4;

        int sum = 0;

        // TODO: add to other player, not self
        int chainingGarbage = 0;    // pending during chaining process
        int garbage = 1;            // ready to be dropped

        // Garbage points leftover from last chain
        bool popLastTurn = false;
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
}