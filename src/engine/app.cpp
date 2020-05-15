#include "app.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <chrono>
#include <iostream>
#include <thread>

#include "loader.hpp"

// #ifdef __APPLE__
// #include <OpenGL/gl.h>
// #include <OpenGL/glext.h>
// #elif _WIN32 || _WIN64
// #include <SDL2/SDL_opengl.h>
// #else
// #include <SDL2/SDL_opengles2.h>
// #endif

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

// void handle_resize() {
//     context_initialized = false;
//     SDL_GL_GetDrawableSize(window, &width, &height);
//     // glViewport(0, 0, width, height);
// }

Application::Application()
    : _frame(0), _running(false), _visible(false), _game(nullptr) {
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

    // Make the largest window possible with an integer scale factor
    // SDL_Rect bounds;
    // SDL::check(SDL_GetDisplayUsableBounds(0, &bounds));
    // const int scaleFactor = std::max(
    //     1, std::min(bounds.w / SCREEN_WIDTH, bounds.h / SCREEN_HEIGHT));

    const int scaleFactor = 1;

    SDL::Window window{SDL::check(
        SDL_CreateWindow("Puyo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         SCREEN_WIDTH * scaleFactor,
                         SCREEN_HEIGHT * scaleFactor, SDL_WINDOW_SHOWN))};

    _renderer = SDL::Renderer{SDL::check(SDL_CreateRenderer(
        window.get(), -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC))};

    SDL::check(
        SDL_RenderSetLogicalSize(_renderer.get(), SCREEN_WIDTH, SCREEN_HEIGHT));

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
                    // case SDL_WINDOWEVENT_SIZE_CHANGED: {
                    //     handle_resize();
                    //     break;
                    // }
            }
        }
    }

    if (!_game->logic()) {
        _running = false;
    }

    if(_visible){
        SDL::check(SDL_SetRenderDrawColor(_renderer.get(), 0x06, 0x16, 0x39, 255));
        SDL::check(SDL_RenderClear(_renderer.get()));

        // // Fill the surface white
        // SDL_FillRect(screenSurface, NULL,
        //              SDL_MapRGB(screenSurface->format, 0x06, 0x16, 0x39));

        // // Update the surface
        // SDL_UpdateWindowSurface(window);

        _game->render(_renderer, _frame);
        ++_frame;

        // // glClear(GL_COLOR_BUFFER_BIT);
        // // for (auto layer : layers) {
        // //     layer->Render(window, !context_initialized);
        // // }

        // context_initialized = true;
        // SDL_GL_SwapWindow(window);

        SDL_RenderPresent(_renderer.get());
    }
}