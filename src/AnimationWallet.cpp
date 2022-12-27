#include "AnimationWallet.hpp"

AnimationWallet::AnimationWallet(std::string dolphin_path)
{
    fs::path search_dir = dolphin_path;
    std::error_code ec;
    if(fs::is_directory(search_dir, ec))
    {
        for(const auto& entry : fs::directory_iterator(dolphin_path))
        {
            const auto filenameStr = entry.path().filename().string();
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
    }
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
