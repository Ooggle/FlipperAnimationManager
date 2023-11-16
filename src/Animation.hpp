#ifndef ANIMATION_H
#define ANIMATION_H

#include <stdint.h>
#include <chrono>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <SDL_opengl.h>

extern "C" {
    #include "heatshrink_decoder.h"
}

typedef enum 
{ 
    BM,
    PNG
} anim_format;

class Animation
{
    private:
        bool valid_animation = false;
        int current_frame_number = 0;
        int total_frames_files = 0;
        GLuint* frames = NULL;
        uint8_t** frames_pixels = NULL;
        float time_per_frame;
        std::vector<int> frames_order;
        std::chrono::system_clock::time_point time_at_last_frame;

        uint passive_frames = 0;
        uint active_frames = 0;

    public:
        bool selected = false;
        bool active = false;

        int min_butthurt = 0;
        int max_butthurt = 14;
        int min_level = 1;
        int max_level = 30;
        int weight = 8;

        anim_format format = BM;
        std::string anim_folder;
        std::string anim_name;

    private:
        void load_animation();
        void next_frame();
        bool read_frames_from_files();
        bool LoadImageFromFile(std::string filename, int file_number);

    public:
        Animation(std::string anim_folder);
        Animation(std::string anim_folder, std::string anim_name);
        ~Animation();
        bool is_valid();
        void reload_animation();
        GLuint get_frame();
        int get_current_frame_number();
        int get_total_frames_number();
        int get_total_frames_files();
        int get_weight();
        void export_to_bm();
};

#endif
