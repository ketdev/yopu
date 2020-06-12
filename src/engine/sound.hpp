#pragma once
#include "utils/sdl_utils.h"

struct Sound {    
    static void play(const SDL::MixMusic& music) {
        Mix_PlayMusic(music.get(), 0);
    }
};