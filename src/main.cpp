#include <SDL2/SDL_main.h>

#include <exception>
#include <iostream>

#include "engine/app.hpp"
#include "game/game.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

const int SCREEN_WIDTH = 2688; // 1080;
const int SCREEN_HEIGHT = 1242; // 1920;

extern "C" int main(int, char *[]) {
#ifdef _WIN32
    // No blurry DPI zoom
    SetProcessDPIAware();
#endif
    try {
        Application app("Puyo", SCREEN_WIDTH, SCREEN_HEIGHT);
        Game game;
        app.run(&game);
    } catch (std::exception &e) {
        std::cout << e.what() << '\n';
        return 1;
    }
    return 0;
}
