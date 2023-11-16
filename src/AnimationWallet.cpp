#include "AnimationWallet.hpp"

AnimationWallet::AnimationWallet()
{
    this->not_initialized = true;
}

AnimationWallet::AnimationWallet(std::string dolphin_path)
{
    this->dolphin_path = dolphin_path;
    this->not_initialized = false;
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

            // generate animation name as in manifest file
            std::string filenameStr;
            if(dolphin_path.at(dolphin_path.size() - 1) == '/' || dolphin_path.at(dolphin_path.size() - 1) == '\\')
                filenameStr = full_path.substr(dolphin_path.length(), full_path.length() - dolphin_path.length());
            else
                filenameStr = full_path.substr(dolphin_path.length() + 1, full_path.length() - dolphin_path.length() + 1);
            
            // replace \ by / (for Windows)
            std::string search = "\\";
            std::string replace = "/";
            size_t pos = 0;
            while((pos = filenameStr.find(search, pos)) != std::string::npos)
            {
                filenameStr.replace(pos, search.length(), replace);
                pos += replace.length();
            }

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
                    // temporarily store animation path and name for future loading
                    this->animations_names_temp.push_back(std::make_pair(full_path, filenameStr));
                    /* Animation* anim = new Animation(full_path, filenameStr);
                    if(anim->is_valid())
                    {
                        this->animations.push_back(anim);
                        this->animations_number+= 1;
                    }
                    else
                    {
                        delete(anim);
                    } */
                }
            }
        }

        this->total_animations__loading = this->animations_names_temp.size();

        if(dolphin_path.at(dolphin_path.size() - 1) == '/' || dolphin_path.at(dolphin_path.size() - 1) == '\\')
            this->manifest_path = dolphin_path + "manifest.txt";
        else
            this->manifest_path = dolphin_path + "/" + "manifest.txt";
    }
    else
        this->is_folder_correct = false;
}

AnimationWallet::~AnimationWallet()
{
    for(Animation* anim: this->animations)
        delete(anim);
    if(this->manifest_created)
        delete(this->manifest);
}

void AnimationWallet::add_animation(std::string anim_folder, std::string anim_name)
{
    Animation* anim = new Animation(anim_folder, anim_name);
    if(anim->is_valid())
    {
        this->animations.push_back(anim);
        this->animations_number+= 1;
    }
    else
    {
        this->errored_animations.push_back(anim_folder);
        delete(anim);
    }
}

void AnimationWallet::parse_manifest()
{
    // Parsing Manifest.txt
    this->manifest = new Manifest(this->manifest_path);
    this->manifest_created = true;

    if(!this->manifest->is_good) return;

    for(Manifest_animation man_anim : this->manifest->manifest_animations)
    {
        for(Animation* anim : this->animations)
        {
            if(anim->anim_name == man_anim.name)
            {
                // apply animation settings from manifest.txt
                anim->selected = true;
                anim->min_butthurt = man_anim.min_butthurt;
                anim->max_butthurt = man_anim.max_butthurt;
                anim->min_level = man_anim.min_level;
                anim->max_level = man_anim.max_level;
                anim->weight = man_anim.weight;
            }
        }
    }
}

bool AnimationWallet::get_is_folder_correct()
{
    return this->is_folder_correct;
}

bool AnimationWallet::update_manifest(std::string file_content)
{
    return this->manifest->update_manifest(file_content);
}

bool AnimationWallet::is_finished_loading()
{
    return this->number_animations_loaded == (int)this->animations_names_temp.size();
}

void AnimationWallet::load_new_animation()
{
    if(this->is_finished_loading())
        return;
    
    this->add_animation(this->animations_names_temp.at(this->number_animations_loaded).first, this->animations_names_temp.at(this->number_animations_loaded).second);
    this->number_animations_loaded+= 1;
}

void AnimationWallet::replace_weight(int new_weight)
{
    for(Animation* anim : this->animations)
        // replace all weight
        anim->weight = new_weight;
}

void AnimationWallet::replace_weight(int new_weight, int old_weight)
{
    for(Animation* anim : this->animations)
    {
        // replace if it match old_weight
        if(anim->weight == old_weight)
            anim->weight = new_weight;
    }
}

void AnimationWallet::replace_min_max_level(int new_min_level, int new_max_level)
{
    for(Animation* anim : this->animations)
    {
        anim->min_level = new_min_level;
        anim->max_level = new_max_level;
    }
}

void AnimationWallet::replace_min_max_butthurt(int new_min_butthurt, int new_max_butthurt)
{
    for(Animation* anim : this->animations)
    {
        anim->min_butthurt = new_min_butthurt;
        anim->max_butthurt = new_max_butthurt;
    }
}

void AnimationWallet::set_active(bool active)
{
    for(Animation* anim : this->animations)
        anim->active = active;
}
