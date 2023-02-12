#include "Animation.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Animation::Animation(std::string anim_folder) : time_at_last_frame(std::chrono::system_clock::now()), anim_folder(anim_folder), anim_name(anim_folder)
{
    this->load_animation();
}

Animation::Animation(std::string anim_folder, std::string anim_name) : time_at_last_frame(std::chrono::system_clock::now())
{
    this->anim_folder = anim_folder;
    this->anim_name = anim_name;
    this->load_animation();
}

void Animation::load_animation()
{
    std::string line;
    std::ifstream meta_file;
    this->current_frame_number = 0;
    this->total_frames_number = 0;
    this->total_frames_files = 0;
    this->time_at_last_frame = std::chrono::system_clock::now();
    meta_file.open((this->anim_folder + std::string("meta.txt")).c_str());

    if(!meta_file.is_open()) {
        perror("Error: open meta.txt");
        return;
    }

    bool total_frames_ok = false;
    bool time_per_frame_ok = false;
    bool frames_ok = false;

    size_t found = 0;
    while(getline(meta_file, line)) {
        // get frame rate
        found = line.find("Frame rate: ");
        if (found != std::string::npos)
        {
            try
            {
                this->time_per_frame = 1 / (float)std::stoi(line.substr(12, line.length() - 12).c_str());
                time_per_frame_ok = 1;
            }
            catch(const std::exception& e) {}
        }

        // get frames
        found = line.find("Frames order: ");
        if (found != std::string::npos)
        {
            try
            {
                std::string raw_frames_order((line.substr(13, line.length() - 13)).c_str());

                size_t start;
                size_t end = 0;
            
                while((start = raw_frames_order.find_first_not_of(' ', end)) != std::string::npos)
                {
                    end = raw_frames_order.find(' ', start);
                    this->frames_order.push_back(std::stoi(raw_frames_order.substr(start, end - start)));
                }

                for(int frame: this->frames_order)
                {
                    if(frame >= this->total_frames_files)
                        this->total_frames_files = frame + 1;
                }
                total_frames_ok = 1;

                if(this->read_frames_from_files())
                    frames_ok = 1;
            }
            catch(const std::exception& e) {}
        }
    }

    meta_file.close();
    
    if(total_frames_ok && time_per_frame_ok && frames_ok)
        this->valid_animation = 1;
    else
    {
        //printf("Error for animation %s\n", anim_folder.c_str());
    }
}

void Animation::reload_animation()
{
    this->valid_animation = 0;
    this->load_animation();
}

Animation::~Animation()
{
    // TODO: delete textures before someone detect that there is a little leakage of memory GPU side
    if(this->frames != NULL)
        free(this->frames);
    if(this->frames_pixels != NULL)
    {
        for(int i = 0; i < this->total_frames_files; i++)
        {
            if(this->frames_pixels[i] != 0)
            {
                free(this->frames_pixels[i]);
            }
        }
    }
}

void Animation::next_frame()
{
    this->current_frame_number+= 1;
    if(this->current_frame_number >= (int)this->frames_order.size())
    {
        this->current_frame_number = 0;
    }
}

bool Animation::read_frames_from_files()
{
    // checks if the animation has bm or png files
    std::ifstream frame_file;
    frame_file.open(this->anim_folder + "frame_0.bm");
    if(frame_file.is_open())
    {
        this->format = BM;
        frame_file.close();
    }
    else
    {
        frame_file.open(this->anim_folder + "frame_0.png");
        if(frame_file.is_open())
        {
            this->format = PNG;
            frame_file.close();
        }
        else
        {
            return false;
        }
    }

    // if a good format is found, proceed to load the files into textures
    this->frames = (GLuint*)malloc(this->total_frames_files * sizeof(GLuint));
    this->frames_pixels = (uint8_t**)malloc(this->total_frames_files * sizeof(uint8_t*));
    memset(this->frames_pixels, 0, this->total_frames_files * sizeof(uint8_t*));
    for(int i = 0; i < this->total_frames_files; i++)
    {
        if(!this->LoadImageFromFile(this->anim_folder + "frame_" + std::to_string(i), i))
            return false;
    }
    
    return true;
}

bool Animation::LoadImageFromFile(std::string filename, int file_number)
{
    // Image dimensions
    int image_width = 128;
    int image_height = 64;
    uint8_t* image_data = NULL;

    if(this->format == BM)
    {
        filename+= ".bm";
        // Load from file
        FILE *f = fopen(filename.c_str(), "rb");
        if(!f)
        {
            printf("Failed opening file\n");
            return false;
        }
        fseek(f, 0, SEEK_END);
        int len = ftell(f);
        fseek(f, 0, SEEK_SET);
        uint8_t* buffer = (uint8_t*)malloc(len+1);
        uint8_t* out_buff = (uint8_t*)malloc(1024);
        fread(buffer, sizeof(char), len, f);
        fclose(f);

        uint8_t* good_buffer = NULL;
        if(buffer[0] == 1)
        {
            // Decompress using lzss heatshrink lib
            good_buffer = buffer + 4;
            int good_len = len - 4;

            // Decode bm file
            heatshrink_decoder* decoder = heatshrink_decoder_alloc(good_len, 8, 4);
            size_t input_size;
            HSD_sink_res sink_res = heatshrink_decoder_sink(decoder, (uint8_t *)good_buffer, (size_t)good_len, &input_size);
            if(sink_res) {}

            HSD_poll_res poll_res;
            poll_res = heatshrink_decoder_poll(decoder, (uint8_t *)out_buff, 1024, &input_size);
            if(poll_res) {}
            heatshrink_decoder_finish(decoder);
            heatshrink_decoder_free(decoder);

            image_data = (uint8_t*)malloc((1024*8)*4);
            int pos = 0;
            // 1024: (128*64) / 8
            int count_bytes = (image_width * image_height) / 8;
            for(int i = 0; i < count_bytes; i++)
            {
                for(int j = 0; j < 8; j++)
                {
                    bool pix = (out_buff[i] >> j) & 0b1;
                    if(pix == 1)
                    {
                        image_data[pos] = 0;
                        pos+= 1;
                        image_data[pos] = 0;
                        pos+= 1;
                        image_data[pos] = 0;
                        pos+= 1;
                    }
                    else
                    {
                        image_data[pos] = 255;
                        pos+= 1;
                        image_data[pos] = 141;
                        pos+= 1;
                        image_data[pos] = 0;
                        pos+= 1;
                    }
                    image_data[pos] = 255;
                    pos+= 1;
                }
            }

        }
        else if(buffer[0] == 0)
        {
            good_buffer = buffer + 1;

            image_data = (uint8_t*)malloc((1024*8)*4);
            int pos = 0;
            for(int i = 0; i < 1024; i++)
            {
                for(int j = 0; j < 8; j++)
                {
                    bool pix = (good_buffer[i] >> j) & 0b1;
                    if(pix == 1)
                    {
                        image_data[pos] = 0;
                        pos+= 1;
                        image_data[pos] = 0;
                        pos+= 1;
                        image_data[pos] = 0;
                        pos+= 1;
                    }
                    else
                    {
                        image_data[pos] = 255;
                        pos+= 1;
                        image_data[pos] = 141;
                        pos+= 1;
                        image_data[pos] = 0;
                        pos+= 1;
                    }
                    image_data[pos] = 255;
                    pos+= 1;
                }
            }
        }
        else
            // Houston??
            return false;

        free(buffer);
        free(out_buff);
    }
    else if(this->format == PNG)
    {
        filename+= ".png";
        image_data = stbi_load(filename.c_str(), &image_width, &image_height, NULL, 4);
        if((image_data == NULL) || (image_width > 128 || image_height > 64))
            return false;
    }
    else
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    //free(image_data);

    this->frames_pixels[file_number] = image_data;
    this->frames[file_number] = image_texture;

    return true;
}

GLuint Animation::get_frame()
{
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - this->time_at_last_frame;
    if(elapsed_seconds.count() > this->time_per_frame)
    {
        this->time_at_last_frame = end;
        this->next_frame();
    }
    return this->frames[this->frames_order[this->current_frame_number]];
}

bool Animation::is_valid()
{
    return this->valid_animation;
}

int Animation::get_current_frame_number()
{
    return this->current_frame_number;
}

int Animation::get_total_frames_number()
{
    return this->total_frames_number;
}

int Animation::get_total_frames_files()
{
    return this->total_frames_files;
}

int Animation::get_weight()
{
    return this->weight;
}

void Animation::export_to_bm()
{
    if(this->format == BM)
        return;

    std::string export_path = this->anim_folder.substr(0, this->anim_folder.length() - 1) + std::string("_compiled");
    std::filesystem::create_directory(export_path);
    if(std::filesystem::exists(export_path + std::string("/meta.txt")))
        std::filesystem::remove(export_path + std::string("/meta.txt"));
    std::filesystem::copy_file(this->anim_folder + std::string("meta.txt"), export_path + std::string("/meta.txt"));

    int image_width = 128;
    int image_height = 64;

    for(int f = 0; f < this->total_frames_files; f++)
    {
        uint8_t* bm_frame = (uint8_t*)malloc(((image_width * image_height) / 8) + 1);
        bm_frame[0] = 0;
        int bm_frame_pos = 1;

        int L, P;
        int pixel_pos = 0;
        uint8_t byte_buffer = 0;
        int byte_buffer_count = 0;
        int count_bytes = (image_width * image_height) * 4;
        for(pixel_pos = 0; pixel_pos < count_bytes; pixel_pos+= 4)
        {
            // for each pixel

            // turn an RGB color into grayscale based on the luminosity of each primary colors, don't ask me why, I took it from here:
            // https://github.com/python-pillow/Pillow/blob/main/src/PIL/Image.py#L905
            L = this->frames_pixels[f][pixel_pos] * 299/1000 +
                this->frames_pixels[f][pixel_pos+1] * 587/1000 +
                this->frames_pixels[f][pixel_pos+2] * 114/1000;
            // i'm inverting black and white while converting to bitmap for optimization sake
            if(L > 127)
                P = 0;
            else
                P = 1;

            byte_buffer+= P << byte_buffer_count;
            byte_buffer_count+= 1;

            if(byte_buffer_count == 8)
            {
                // push the completed byte to the output array
                bm_frame[bm_frame_pos] = byte_buffer;
                byte_buffer = 0;
                bm_frame_pos+= 1;
                byte_buffer_count = 0;
            }
        }
        printf("\n");
        std::ofstream manifest_file;
        manifest_file.open((export_path + std::string("/frame_") + std::to_string(f) + std::string(".bm")).c_str(), std::ofstream::trunc | std::ofstream::binary);

        if(!manifest_file.is_open()) {
            perror("Error: open frame");
            break;
        }

        // TODO: add the lzss heatshrink compression to final frame and check if it's size is larger or not than the original uncompressed frame:
        // https://github.com/flipperdevices/flipperzero-firmware/blob/dev/scripts/flipper/assets/icon.py#L107
        manifest_file.write((const char *)bm_frame, ((image_width * image_height) / 8) + 1);

        manifest_file.close();
    }
}
