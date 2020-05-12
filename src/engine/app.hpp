#pragma once
#include "sdl_utils.hpp"
#include "game.hpp"

class Application {
   public:
    Application();
    ~Application();

    void run(Game* game);

   private:
    uint64_t _frame;
    bool _running;
    bool _visible;
    void _loop();

    SDL::Renderer _renderer;
    Game* _game;

#ifdef __EMSCRIPTEN__
    friend void app_loop_wrap(Application*);
#endif
};