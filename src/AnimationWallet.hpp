#ifndef ANIMATIONWALLET_H
#define ANIMATIONWALLET_H

#include <string>
#include <vector>
#include <utility>
#include <filesystem>
#include "Animation.hpp"
#include "Manifest.hpp"

namespace fs = std::filesystem;

class AnimationWallet
{
    public:
        bool not_initialized = false;
        int animations_number = 0;
        int total_animations__loading = 0;
        std::vector<Animation*> animations;
        std::string manifest_path;
        std::vector<std::string> errored_animations;
        Manifest* manifest;
        bool manifest_created = false;

    private:
        bool is_folder_correct = false;
        std::string dolphin_path;
        int number_animations_loaded = 0;
        std::vector<std::pair<std::string, std::string>> animations_names_temp;

    public:
        AnimationWallet();
        AnimationWallet(std::string dolphin_path);
        ~AnimationWallet();
        void load_animations();
        void add_animation(std::string anim_folder, std::string anim_name);
        void parse_manifest();
        bool get_is_folder_correct();
        bool update_manifest(std::string file_content);
        bool is_finished_loading();
        void load_new_animation();
        void replace_weight(int new_weight);
        void replace_weight(int new_weight, int old_weight);
        void replace_min_max_level(int new_min_level, int new_max_level);
        void replace_min_max_butthurt(int new_min_butthurt, int new_max_butthurt);
};

#endif
