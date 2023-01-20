#ifndef ANIMATIONWALLET_H
#define ANIMATIONWALLET_H

#include <string>
#include <vector>
#include <filesystem>
#include "Animation.hpp"
#include "Manifest.hpp"

namespace fs = std::filesystem;

class AnimationWallet
{
    public:
        int animations_number = 0;
        std::vector<Animation*> animations;
        Manifest* manifest;

    private:
        bool is_folder_correct = false;
        std::string dolphin_path;

    public:
        AnimationWallet(std::string dolphin_path = std::string(""));
        ~AnimationWallet();
        void load_animations();
        void add_animation(std::string anim_folder);
        bool get_is_folder_correct();
        bool update_manifest(std::string file_content);
};

#endif
