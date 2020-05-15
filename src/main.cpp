#include <SDL2/SDL_main.h>

#include <exception>
#include <iostream>

#include "engine/app.hpp"
#include "game/game.hpp"

extern "C" int main(int, char *[]) {
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
