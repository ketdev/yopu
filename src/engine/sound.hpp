#pragma once
#include "sdl_utils.hpp"

struct Sound {    
    static void play(const SDL::MixMusic& music) {
        Mix_PlayMusic(music.get(), 0);
    }
};