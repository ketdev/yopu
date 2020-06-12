#pragma once
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_mixer.h>

#include <stdexcept>

#include "clean_ptr.h"

namespace SDL {

using Window = std::clean_ptr<SDL_Window, SDL_DestroyWindow>;
using GlContext = std::clean_vptr<SDL_GLContext, SDL_GL_DeleteContext>;

using Texture = std::clean_ptr<SDL_Texture, SDL_DestroyTexture>;
using Surface = std::clean_ptr<SDL_Surface, SDL_FreeSurface>;
using Renderer = std::clean_ptr<SDL_Renderer, SDL_DestroyRenderer>;
using MixMusic = std::clean_ptr<Mix_Music, Mix_FreeMusic>;

[[noreturn]] inline void raise() {
    const char *error = SDL_GetError();
    throw std::runtime_error{error};
}

template <typename Ptr>
[[nodiscard]] Ptr *check(Ptr *ptr) {
    if (ptr == nullptr) raise();
    return ptr;
}

inline int check(const int code) {
    if (code < 0) raise();
    return code;
}

}  // namespace SDL