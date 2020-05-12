#include <SDL2/SDL_main.h>

#include <exception>
#include <iostream>

#include "engine/app.hpp"
#include "game/boardgame.hpp"

extern "C" int main(int, char *[]) {
    try {
        Application app;
        BoardGame game;
        app.run(&game);
    } catch (std::exception &e) {
        std::cout << e.what() << '\n';
        return 1;
    }
    return 0;
}
