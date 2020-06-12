#pragma once
#include <string>
#include <SDL2/SDL_image.h>
#include "utils/sdl_utils.h"

struct Loader {
    static SDL::Texture loadTexture(SDL::Renderer& renderer, std::string path) {
        SDL::Surface surface{SDL::check(IMG_Load(path.c_str()))};
        return SDL::Texture{SDL::check(
            SDL_CreateTextureFromSurface(renderer.get(), surface.get()))};
    }

    static SDL::MixMusic loadMusic(std::string path) {
        SDL::MixMusic music{ SDL::check(Mix_LoadMUS(path.c_str())) };
        return music;
    }
};