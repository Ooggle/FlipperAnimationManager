#include "AnimationWallet.hpp"

AnimationWallet::AnimationWallet(std::string dolphin_path)
{
    this->dolphin_path = dolphin_path;
    this->load_animations();
}

void AnimationWallet::load_animations()
{
    std::string dolphin_path = this->dolphin_path;

    fs::path search_dir = dolphin_path;
    std::error_code ec;
    if(fs::is_directory(search_dir, ec))
    {
        this->is_folder_correct = true;
        for(const auto& entry : fs::recursive_directory_iterator(dolphin_path))
        {
            std::string full_path = entry.path().string();
            std::string filenameStr;
            if(dolphin_path.at(dolphin_path.size() - 1) == '/' || dolphin_path.at(dolphin_path.size() - 1) == '\\')
                filenameStr = full_path.substr(dolphin_path.length(), full_path.length() - dolphin_path.length());
            else
                filenameStr = full_path.substr(dolphin_path.length() + 1, full_path.length() - dolphin_path.length() + 1);
            if(entry.is_directory()) {
                std::string full_path;
                fs::path f;
                if(dolphin_path.at(dolphin_path.size() - 1) == '/' || dolphin_path.at(dolphin_path.size() - 1) == '\\')
                    full_path = dolphin_path + filenameStr + "/";
                else
                    full_path = dolphin_path + "/" + filenameStr + "/";
                f = full_path + "meta.txt";
                if(fs::exists(f))
                {
                    Animation* anim = new Animation(full_path, filenameStr);
                    if(anim->is_valid())
                    {
                        this->animations.push_back(anim);
                        this->animations_number+= 1;
                    }
                    else
                    {
                        delete(anim);
                    }
                }
            }
        }

        std::string manifest_path;
        if(dolphin_path.at(dolphin_path.size() - 1) == '/' || dolphin_path.at(dolphin_path.size() - 1) == '\\')
            manifest_path = dolphin_path + "manifest.txt";
        else
            manifest_path = dolphin_path + "/" + "manifest.txt";

        // Parsing Manifest.txt
        Manifest* manifest = new Manifest(manifest_path);

        if(!manifest->is_good) return;

        for(Manifest_animation man_anim: manifest->manifest_animations)
        {
            for(Animation* anim: this->animations)
            {
                if(anim->anim_name == man_anim.name)
                {
                    // apply animation settings from manifest.txt
                    anim->selected = true;
                    anim->min_butthurt = man_anim.min_butthurt;
                    anim->max_butthurt =  man_anim.max_butthurt;
                    anim->min_level = man_anim.min_level;
                    anim->max_level =  man_anim.max_level;
                    anim->weight = man_anim.weight;
                }
            }
        }
        free(manifest);
    }
    else
        this->is_folder_correct = false;
}

AnimationWallet::~AnimationWallet()
{
    for(Animation* anim: this->animations)
        delete(anim);
}

void AnimationWallet::add_animation(std::string anim_folder)
{
    Animation* anim = new Animation(anim_folder);
    if(anim->is_valid())
    {
        this->animations.push_back(anim);
        this->animations_number+= 1;
    }
    else
    {
        delete(anim);
    }
}

bool AnimationWallet::get_is_folder_correct()
{
    return this->is_folder_correct;
}
