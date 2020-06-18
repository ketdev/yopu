#pragma once
#include "../../entity.h"

namespace puyo {

	// -- Components --

	struct GravityAnimation {

		// // Pair split constants
		// static constexpr int puyoInitialSpeed = 0x10000;  // (1/32) cell/frame
		// static constexpr int puyoTerminalSpeed = 0x80000; // (8/32) cell/frame
		// static constexpr int puyoAcceleration = 0x03000;  // cells/frame2

		int distance;			// distance to fall
		double speed;			// current fall speed
		double terminalSpeed;	// maximal speed limit
		double acceleration;	// speed acceleration per frame
	};

	/*
	Garbage:
		initial speed: 0 (0 pixel/frame)
		terminal velocity: 0x80000 (8 pixels/frame)
		acceleration for column 1: 0x02400 (0.140625 pixels/frame2)
		acceleration for column 2: 0x02600 (0.1484375 pixels/frame2)
		acceleration for column 3: 0x02000 (0.125 pixels/frame2)
		acceleration for column 4: 0x02A00 (0.1640625 pixels/frame2)
		acceleration for column 5: 0x02200 (0.1328125 pixels/frame2)
		acceleration for column 6: 0x02800 (0.15625 pixels/frame2)
	*/

	// -- Render Systems --

	void gravity(registry& reg);
}