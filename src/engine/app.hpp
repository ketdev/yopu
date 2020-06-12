#pragma once
#include <string>
#include <memory>
#include "utils/sdl_utils.h"
#include "igame.hpp"

#include "system/render.h"

class Application {
public:
    Application(const std::string& title, int width, int height);
    ~Application();

    void run(IGame* game);

private:
    const std::string _title;
    int _width;
    int _height;
    uint64_t _frame;
    bool _running;
    bool _visible;

    SDL::Window _window;
    SDL::GlContext _context;
    SDL::Renderer _renderer;
    IGame* _game;

    // Default systems
    std::unique_ptr<Render> _render;

private:
    void _loop();

#ifdef __EMSCRIPTEN__
    friend void app_loop_wrap(Application*);
#endif
};