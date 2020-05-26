#include "sound.h"
#include "../entity.h"
#include "../../engine/sound.hpp"
#include "../../engine/loader.hpp"

void media::soundPlayer(registry& reg) {
    // Sound assets
    // TODO: move to central loader, must be created and released after application
    static SDL::MixMusic sounds[int(media::SoundEffect::_Count)] = {
        Loader::loadMusic("src/assets/sound/move.ogg"),
        Loader::loadMusic("src/assets/sound/rotate.ogg"),
        Loader::loadMusic("src/assets/sound/drop.ogg"),
    };

    auto view = reg.view<media::SoundEffect>();
    for (auto& sfx : view) {
        auto i = int(view.get<media::SoundEffect>(sfx));
        Sound::play(sounds[i]);
        reg.remove<media::SoundEffect>(sfx);
        reg.destroy(sfx);
    }
}