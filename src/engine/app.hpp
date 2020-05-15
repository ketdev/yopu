#pragma once
#include "sdl_utils.hpp"
#include "igame.hpp"

class Application {
   public:
    Application();
    ~Application();

    void run(IGame* game);

   private:
    uint64_t _frame;
    bool _running;
    bool _visible;
    void _loop();

    SDL::Renderer _renderer;
    IGame* _game;

#ifdef __EMSCRIPTEN__
    friend void app_loop_wrap(Application*);
#endif
};