#pragma once

class no_copy {
   protected:
    no_copy() = default;
    ~no_copy() = default;
    no_copy(const no_copy&) = delete;
    no_copy& operator=(const no_copy&) = delete;
};
