#include "Animation.hpp"

Animation::Animation(std::string anim_folder) : anim_folder(anim_folder), time_at_last_frame(std::chrono::system_clock::now())
{
    this->load_animation(anim_folder);
}

void Animation::load_animation(std::string anim_folder)
{
    std::string line;
    std::ifstream meta_file;
    this->anim_name = anim_folder; // temp
    this->current_frame_number = 0;
    this->total_frames_number = 0;
    this->total_frames_files = 0;
    this->time_at_last_frame = std::chrono::system_clock::now();
    meta_file.open((anim_folder + std::string("meta.txt")).c_str());

    if(!meta_file.is_open()) {
        perror("Error: open meta.txt");
        return;
    }

    bool total_frames_ok = 0;
    bool time_per_frame_ok = 0;
    bool frames_ok = 0;

    size_t found = 0;
    while(getline(meta_file, line)) {
        // get frame rate
        found = line.find("Frame rate: ");
        if (found != std::string::npos)
        {
            try
            {
                this->time_per_frame = 1 / (float)std::stoi(line.substr(12, line.length() - 12).c_str());
                //printf("this->time_per_frame: %f\n", this->time_per_frame);
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
                //this->time_per_frame = 1 / (float)std::stoi(line.substr(12, line.length() - 12).c_str());
                std::string raw_frames_order((line.substr(13, line.length() - 13)).c_str());

                size_t start;
                size_t end = 0;
            
                while((start = raw_frames_order.find_first_not_of(' ', end)) != std::string::npos)
                {
                    end = raw_frames_order.find(' ', start);
                    this->frames_order.push_back(std::stoi(raw_frames_order.substr(start, end - start)));
                }

                for (int frame: this->frames_order) {
                    if(frame >= this->total_frames_files)
                        this->total_frames_files = frame + 1;
                }
                total_frames_ok = 1;

                if(this->read_frames_from_files() == 0)
                    frames_ok = 1;
            }
            catch(const std::exception& e) {}
        }
    }
    
    if(total_frames_ok && time_per_frame_ok && frames_ok)
        this->valid_animation = 1;
    else
    {
        printf("Error for animation %s\n", anim_folder.c_str());
    }
}

void Animation::reload_animation()
{
    this->valid_animation = 0;
    this->load_animation(this->anim_folder);
}

Animation::~Animation()
{
    if(this->frames != NULL)
        free(this->frames);
}

void Animation::next_frame()
{
    this->current_frame_number+= 1;
    if(this->current_frame_number >= this->total_frames_files)
    {
        this->current_frame_number = 0;
    }
}

bool Animation::read_frames_from_files()
{
    //printf("total_frames_files: %d\n", this->total_frames_files);
    this->frames = (GLuint*)malloc(this->total_frames_files * sizeof(GLuint));
    for(int i = 0; i < this->total_frames_files; i++)
    {
        bool ret = this->LoadBmFromFile(this->anim_folder + "frame_" + std::to_string(i) + ".bm", i);
        //printf("ret: %d\n", ret);
        if(ret != 0)
            return -1;
    }
    return 0;
}

bool Animation::LoadBmFromFile(std::string filename, int file_number)
{
    //printf("CALL LoadBmFromFile with filename: %s and file_number: %d\n", filename.c_str(), file_number);
    // Load from file
    FILE *f = fopen(filename.c_str(), "rb");
    if(!f)
    {
        printf("Failed opening file\n");
        return -1;
    }
    fseek(f, 0, SEEK_END);
    int len = ftell(f);
    //printf("Size: %d\n", len);
    fseek(f, 0, SEEK_SET);
    unsigned char* buffer = (unsigned char*)malloc(len+1);
    unsigned char* out_buff = (unsigned char*)malloc(1024);
    fread(buffer, sizeof(char), len, f);
    fclose(f);

    unsigned char* good_buffer = NULL;
    unsigned char* image_data = NULL;
    //printf("data: %d\n", buffer[0]);
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
        //printf("size consumed: %ld\n", input_size);

        HSD_poll_res poll_res;
        poll_res = heatshrink_decoder_poll(decoder, (uint8_t *)out_buff, 1024, &input_size);
        if(poll_res) {}
        heatshrink_decoder_finish(decoder);
        heatshrink_decoder_free(decoder);

        image_data = (unsigned char*)malloc((1024*8)*4);
        int pos = 0;
        int col = 0;
        for(int i = 0; i < 1024; i++)
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
        good_buffer = buffer + 2;
        int good_len = len - 2;

        image_data = (unsigned char*)malloc((1024*8)*4);
        int pos = 0;
        int col = 0;
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
        return -1;

    free(buffer);
    free(out_buff);

    // Image dimensions
    int image_width = 128;
    int image_height = 64;

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
    free(image_data);

    this->frames[file_number] = image_texture;
    //printf("image_texture: %d\n", image_texture);

    return 0;
}

GLuint Animation::get_frame()
{
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - this->time_at_last_frame;
    //printf("elapsed_seconds.count(): %f\n", elapsed_seconds.count());
    if(elapsed_seconds.count() > this->time_per_frame)
    {
        this->time_at_last_frame = end;
        this->next_frame();
    }
    return this->frames[this->current_frame_number];
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