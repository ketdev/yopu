#include "sprite.h"
#include "../../engine/loader.hpp"

void media::spriteRender(registry& reg, SDL::Renderer& renderer) {
    // Texture assets
    // TODO: move to central loader, must be created and released after application
    static SDL::Texture textures[int(media::Texture::_Count)] = {
        Loader::loadTexture(renderer, "src/assets/texture/background.png"),
    };

    auto view = reg.view<media::Texture>();
    for (auto& tex : view) {
        auto& sdlTexture = textures[int(view.get<media::Texture>(tex))];

        auto pos = reg.try_get<media::Position>(tex);

        Uint32 format;
        int access, w, h;
        SDL::check(SDL_QueryTexture(sdlTexture.get(), &format, &access, &w, &h));

        SDL_Rect src = { 0, 0, w, h };
        SDL_Rect dst = { pos ? pos->x : 0, pos ? pos->y : 0, w, h };
        SDL_RenderCopy(renderer.get(), sdlTexture.get(), &src, &dst);
    }
}