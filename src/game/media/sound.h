#pragma once
#include <game/object/object.h>

namespace media {

	// -- Components --

	enum class SoundEffect {
		Move = 0, 
		Rotate,
		Drop,

		_Count,
	};


	// -- Systems --

	void soundPlayer(registry& reg);


	// -- Util --

	static inline void play(registry& reg, SoundEffect sfx) {
		reg.emplace<media::SoundEffect>(reg.create(), sfx);
	}
}