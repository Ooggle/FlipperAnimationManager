#include "AnimationWallet.hpp"

AnimationWallet::AnimationWallet()
{
    
}

AnimationWallet::~AnimationWallet()
{

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
