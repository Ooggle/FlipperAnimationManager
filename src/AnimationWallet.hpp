#ifndef ANIMATIONWALLET_H
#define ANIMATIONWALLET_H

#include <string>
#include <vector>
#include <filesystem>
#include "Animation.hpp"

namespace fs = std::filesystem;

class AnimationWallet
{
    public:
        int animations_number = 0;
        std::vector<Animation*> animations;

    private:

    public:
        AnimationWallet(std::string dolphin_path = std::string(""));
        ~AnimationWallet();
        void add_animation(std::string anim_folder);
};

#endif
