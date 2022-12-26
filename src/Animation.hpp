#ifndef ANIMATION_H
#define ANIMATION_H

#include <chrono>
#include <string>
#include <vector>
#include <fstream>
#include <SDL_opengl.h>

extern "C" {
    #include "heatshrink_decoder.h"
}

class Animation
{
    private:
        bool valid_animation = 0;
        int current_frame_number = 0;
        int total_frames_number = 0;
        int total_frames_files = 0;
        GLuint* frames = NULL;
        float time_per_frame;
        std::vector<int> frames_order;
        std::chrono::system_clock::time_point time_at_last_frame;

    public:
        bool selected = 0;
        std::string anim_name;
        std::string anim_folder;

    private:
        void next_frame();
        bool read_frames_from_files();
        bool LoadBmFromFile(std::string filename, int file_number);

    public:
        Animation(std::string anim_folder);
        ~Animation();
        GLuint get_frame();
        bool is_valid();
};

#endif
