#pragma once
#include <stdint.h>
#include <entt\entt.hpp>

#include "player\board.h"
#include "player\spawner.h"

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
