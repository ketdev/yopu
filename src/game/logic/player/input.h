#pragma once
#include "../../entity.h"

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

	// -- Systems --

	void updateInput(registry& reg);
}
