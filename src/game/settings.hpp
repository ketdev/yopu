#pragma once
#include <vector>
#include <SDL2/SDL_scancode.h>
#include <glm/vec2.hpp>
#include "puyo/puyo.h"

// -- Constants --

static const char* ASSET_PATH_TEXTURE = "src/assets/puyos.png";

const int LOCKING_FRAMES = 8;
const int BLINKING_FRAMES = 10;
const int BOUNCING_FRAMES = 16;

// -- Game Settings --

struct Settings {

    // Number of playable colors (3-5)
    int colorCount = 5;

    // 16 frames needed to cross a cell
    int dropSpeed = puyo::DROP_RES / 16;

    //--

    // Input controller interfaces
    std::vector< std::vector<SDL_Scancode> > controllers = {
        { SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_Z, SDL_SCANCODE_X }
    };

    /*
    RE:
        Margin Time	            - Ranges from 0x00 (0 seconds) to 0x3E (992 seconds); default: 0x06 (time period that indicates how long it takes before the target points starts to decrease)
        Chain Type Angle	    - Chain type, angle of animation (0-3, default 4)
        Zenkeshi Ojama Count    - Number of bonus ojama puyos sent by an all-clear (0-255, default 30)
        Ojama Points	        - Points given for clearing an ojama puyo (0-250, default 0, 10 points increments)
        Sousai (Counter mode)   - On (1) / Off (0), default: 1
        Ojama Needed Points     - Needed points to send a single ojama (0-250, default 70 or 120 in custom rules, 10 points increments)
        Puyos to Clear	        - Needed puyos to clear a chain (1-72, default 4)
        Ojama Mode	            - Ojama puyo mode: standard, hard puyos, top row only (0: standard, 1: hard, 2: hard2, default 0)
        Rule Henka (Hard/Point) - On (1) / Off (0), default: 0
        Rensa Sibari	        - Ranges from "Off" (0) to "Limit 9" (8), default: 0 (causes chains with fewer than a set number of steps to score nothing and to send no garbage)
        VS Com. Level	        - Easy (0) / Normal (1) / Hard (2) / Hardest (3), default: 1
    */
};