#pragma once
#include <string>
#include "graphics/texture.h"
#include "utils/sdl_utils.h"

struct Loader {
    static Texture loadTexture(std::string path) {
        Texture tex;
        tex.load(path);
        return tex;
    }

    static SDL::MixMusic loadMusic(std::string path) {
        SDL::MixMusic music{ SDL::check(Mix_LoadMUS(path.c_str())) };
        return music;
    }
};