// #include <stdio.h>

// int main() {
//     printf("hello, world!\n");
//     return 0;
// }

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <unistd.h>
#endif
#include <stdlib.h>

/**
 * Loads the image located at 'fileName' and copies it to the
 * renderer 'renderer'
 */
int testImage(SDL_Renderer *renderer, const char *fileName) {
    SDL_Surface *image = IMG_Load(fileName);
    if (!image) {
        printf("IMG_Load: %s\n", IMG_GetError());
        return 0;
    }
    int result = image->w;

    /**
     * position and size that you wish the image to be copied
     * to on the renderer:
     */
    SDL_Rect dest = {.x = 10, .y = 10, .w = 512, .h = 512};

    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, image);

    SDL_RenderCopy(renderer, tex, NULL, &dest);

    /**
     * Now that the image data is in the renderer, we can free the memory
     * used by the texture and the image surface
     */
    SDL_DestroyTexture(tex);

    SDL_FreeSurface(image);

    return result;
}

int main(int argc, char *args[]) {
// int main() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window;
    SDL_Renderer *renderer;

    SDL_CreateWindowAndRenderer(800, 600, 0, &window, &renderer);

    int result = 0;

    /**
     * Set up a white background
     */
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    /**
     * Load and copy the test image to the renderer
     */
    result |= testImage(renderer, "src/assets/classic_puyos.png");

    /**
     * Show what is in the renderer
     */
    SDL_RenderPresent(renderer);

    printf("you should see an image.\n");

    return 0;
}
