#pragma once
#include <stdint.h>
#include <entt\entt.hpp>
#include <vector>

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

	// When player lost
	struct GameOver {};

	// -- Components --

	struct Input { 
		int controllerIndex = 0;
		struct Key {
			bool next, curr, trigger;
			int counter;
		} keys[InputKey::_Count];
	};

	struct Board { 
		std::vector<std::vector<entt::entity>> grid;

		// The index of the shared spawn pool
		uint32_t spawnPoolIndex = {};

		Board(uint8_t rows, uint8_t cols) {
			grid.resize(cols);
			for (auto& row : grid) {
				row.resize(cols);
			}
		}
	};

}  // namespace player
