#pragma once
#include "../../entity.h"

namespace player {

	// -- Tags --

	// Freefalling stage, waiting for all falling puyos to settle
	struct Freefalling {};

	// -- Systems --

	//  Adds Animation: Bounce
	//  Plays Sound: Drop
	void freefall(registry& reg);

}