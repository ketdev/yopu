#include "animate.h"
#include "puyo.h"
#include "../../media/sound.h"
#include <algorithm>

void puyo::gravity(registry& reg) {
    auto view = reg.view<puyo::RenderPosition, puyo::GravityAnimation>();
    for (auto& puyo : view) {
        auto& pos = view.get<puyo::RenderPosition>(puyo);
        auto& gravity = view.get<puyo::GravityAnimation>(puyo);

        // Apply freefall displacement
        int delta = std::min(static_cast<int>(gravity.speed), gravity.distance);
        pos.y += delta;
        gravity.distance -= delta;

        // Accelerate and cap speed
        gravity.speed = std::min(gravity.speed + gravity.acceleration, gravity.terminalSpeed);

        // Check if reached destination
        if (gravity.distance == 0) {

            // State bouncing animation
            reg.emplace_or_replace<puyo::BounceAnimation>(puyo);

            // State placement sound effect 
            media::play(reg, media::SoundEffect::Drop);

            // Finished with gravity
            reg.remove<puyo::GravityAnimation>(puyo);
        }
    }
}