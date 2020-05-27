#include "input.h"
#include <iostream>

void player::updateInput(registry& _reg) {
    auto view = _reg.view<Input>();
    for (auto& e : view) {
        auto& input = view.get<Input>(e);
        for (size_t i = 0; i < InputKey::_Count; i++) {

            if (input.keys[i].isDown) {
                input.keys[i].counter++;
            }
            else {
                input.keys[i].counter = -1;
            }

            // Calculate repeat triggers
            input.keys[i].repeat =
                // initial press
                (input.keys[i].counter == 0)
                // first repeat
                || (input.keys[i].counter == player::Input::buttonRepeatDelay)
                // subsequent repeats
                || (input.keys[i].counter > player::Input::buttonRepeatDelay
                    && ((input.keys[i].counter - player::Input::buttonRepeatDelay) % player::Input::buttonSubsequentDelay) == 0);
        }

        // Accumulate move input triggers
        input.dx = 0;
        input.dr = 0;

        if (input.keys[player::InputKey::Left].repeat) input.dx--;
        if (input.keys[player::InputKey::Right].repeat) input.dx++;

        if (input.keys[player::InputKey::RotateLeft].counter == 0) input.dr--; // no button repeat, only first press
        if (input.keys[player::InputKey::RotateRight].counter == 0) input.dr++; // no button repeat, only first press

        input.softDrop = input.keys[player::InputKey::Down].isDown; // no repeat, computed on every frame

        if(input.dx || input.dr || input.softDrop)
            std::cout << "Input X:" << input.dx << " R:" << input.dr << " SD:" << input.softDrop << std::endl;
    }

}