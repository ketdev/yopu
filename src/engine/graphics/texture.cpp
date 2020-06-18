#include "texture.h"
#include "../utils/xgl.h"
#include "../utils/sdl_utils.h"
#include <SDL2/SDL_image.h>

inline static GLuint createTexture() {
	GLuint id;
    glGenTextures(1, &id);
    return id;
}

Texture::Texture() : id(createTexture()), width(0), height(0) {}
Texture::~Texture() { glDeleteTextures(1, &id); }

void Texture::load(const std::string& path) {
    SDL::Surface surface{ SDL::check(IMG_Load(path.c_str())) };
    *const_cast<uint32_t*>(&this->width) = surface->w;
    *const_cast<uint32_t*>(&this->height) = surface->h;

    int Mode = GL_RGB;
    if (surface->format->BytesPerPixel == 4) {
        Mode = GL_RGBA;
    }
    use();
    glTexImage2D(GL_TEXTURE_2D, 0, Mode, 
        surface->w, surface->h, 0, Mode, GL_UNSIGNED_BYTE, surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture::use() const{
    glBindTexture(GL_TEXTURE_2D, id);
}