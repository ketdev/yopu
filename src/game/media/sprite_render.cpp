#include "sprite.h"
#include "../../engine/loader.hpp"
#include <set>
#include <list>

struct SortedSprites {
    int layer;
    entity entity;
    bool operator<(const SortedSprites& other) const {
        return layer < other.layer;
    }
};

void media::spriteRender(registry& reg, SDL::Renderer& renderer) {
    // Texture assets
    // TODO: move to central loader, must be created and released after application
    static SDL::Texture textures[int(media::Texture::_Count)] = {
        Loader::loadTexture(renderer, "src/assets/texture/background.png"),
        Loader::loadTexture(renderer, "src/assets/puyos.png"),
    };

    auto view = reg.view<media::Sprite>();
    
    // sort by sprite layer
    std::multiset<SortedSprites> set;
    for (auto& e : view) { set.insert(SortedSprites{ reg.get<media::Sprite>(e).layer, e }); }

    for (auto& se : set) {
        auto& sprite = view.get<media::Sprite>(se.entity);
        auto& sdlTexture = textures[int(sprite.texture)];

        Uint32 format;
        int access, w, h;
        SDL::check(SDL_QueryTexture(sdlTexture.get(), &format, &access, &w, &h));

        SDL_Rect src = { sprite.uv.x * w, sprite.uv.y * h, sprite.uv.z * w, sprite.uv.w * h };
        SDL_Rect dst = { sprite.pos.x, sprite.pos.y, sprite.size.x, sprite.size.y };
        SDL_RenderCopy(renderer.get(), sdlTexture.get(), &src, &dst);
    }
}