#include <SDL2/SDL_main.h>

#include <exception>
#include <iostream>

#include "engine/app.hpp"
#include "game/game.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

extern "C" int main(int, char *[]) {
#ifdef _WIN32
    SetProcessDPIAware();
#endif
    try {
        Application app;
        Game game;
        app.run(&game);
    } catch (std::exception &e) {
        std::cout << e.what() << '\n';
        return 1;
    }
    return 0;
}
