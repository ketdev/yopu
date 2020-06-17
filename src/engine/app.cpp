#include "app.hpp"
#include "loader.hpp"

#include <chrono>
#include <iostream>
#include <thread>
#include <algorithm>

#include "utils/xgl.h"
#include <SDL2/SDL_mixer.h>


Application::Application(const std::string& title, int width, int height)
    : _title(title), _width(width), _height(height), _frame(0), _running(false), _visible(false), _game(nullptr) {
    SDL::check(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO));
    SDL::check(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024));
}

Application::~Application() { 
    Mix_CloseAudio();
    SDL_Quit(); 
}

#ifdef __EMSCRIPTEN__
void app_loop_wrap(Application* app) { app->_loop(); }
#endif

void Application::run(IGame* game) {
    _game = game;

    _window = SDL::Window{SDL::check(
        SDL_CreateWindow(_title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _width, _height, 
            SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI))};

    _context = SDL::GlContext{ SDL::check(SDL_GL_CreateContext(_window.get())) };

#ifdef _WIN32 || _WIN64
    //Initialize GLEW
    glewExperimental = GL_TRUE;
    SDL::check(glewInit());
#endif

    // Initialize openGL settings
    SDL::check(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3));
    SDL::check(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0));
    SDL::check(SDL_GL_SetSwapInterval(1));
    SDL::check(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1));
    SDL::check(SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24));

    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    _renderer = SDL::Renderer{SDL::check(SDL_CreateRenderer(
        _window.get(), -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_PRESENTVSYNC))};

    // SDL::check(
    //     SDL_RenderSetLogicalSize(_renderer.get(), SCREEN_WIDTH, SCREEN_HEIGHT));

    _render.reset(new Render());

    _game->init(_renderer);

    _visible = true;
    _running = true;
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(reinterpret_cast<em_arg_callback_func>(app_loop_wrap), this, 0, 1);
#else
    while (_running) {
        const int fps = 60;
        using Clock = std::chrono::steady_clock;
        auto interval{Clock::duration{Clock::duration::period::den / fps}};
        auto start{Clock::now()};

        _loop();

        std::this_thread::sleep_until(start + interval);
    }
#endif

    return;
}

void Application::_loop() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT: {
                _running = false;
                break;
            }
            case SDL_KEYUP: {
                int sym = e.key.keysym.sym;
                if (sym == SDLK_ESCAPE) {
                    _running = false;
                }
                _game->input(e.key.keysym.scancode, false);
                break;
            }
            case SDL_KEYDOWN: {
                _game->input(e.key.keysym.scancode, true);
                break;
            }
        }
        if (e.type == SDL_WINDOWEVENT) {
            switch (e.window.event) {
                case SDL_WINDOWEVENT_SHOWN: {
                    _visible = true;
                    break;
                }
                case SDL_WINDOWEVENT_HIDDEN: {
                    _visible = false;
                    break;
                }
                case SDL_WINDOWEVENT_SIZE_CHANGED: {
                    _width = e.window.data1;
                    _height = e.window.data2;

                    SDL_GL_GetDrawableSize(_window.get(), &_width, &_height);
                    glViewport(0, 0, _width, _height);
                    break;
                }
            }
        }
    }

    if (!_game->logic()) {
        _running = false;
    }

    if(_visible){
#if 0        
        //SDL::check(SDL_SetRenderDrawColor(_renderer.get(), 0x06, 0x16, 0x39, 255));
        SDL::check(SDL_RenderClear(_renderer.get()));

        _game->render(_renderer, _frame);
        ++_frame;

        SDL_RenderPresent(_renderer.get());
#else
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        _render->draw();

        //_game->render(_renderer, _frame);
        ++_frame;

        SDL_GL_SwapWindow(_window.get());
#endif
    }
}