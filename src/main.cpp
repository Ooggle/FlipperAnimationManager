// Dear ImGui: standalone example application for SDL2 + OpenGL
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

// **DO NOT USE THIS CODE IF YOUR CODE/ENGINE IS USING MODERN OPENGL (SHADERS, VBO, VAO, etc.)**
// **Prefer using the code in the example_sdl_opengl3/ folder**
// See imgui_impl_sdl.cpp for details.

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl2.h"
#include "Animation.hpp"
#include "AnimationWallet.hpp"
#include <stdio.h>
#include <SDL.h>
#include <SDL_opengl.h>
//#include <unistd.h>

extern "C" {
    #include "heatshrink_decoder.h"
}


bool LoadTextureFromFileCustom(const char* filename, GLuint* out_texture, int* out_width, int* out_height)
{
    // Load from file
    FILE *f = stbi__fopen(filename, "rb");
    if (!f) printf("Failed opening file\n");
    fseek(f, 0, SEEK_END);
    int len = ftell(f);
    //printf("Size: %d\n", len);
    fseek(f, 0, SEEK_SET);
    unsigned char* buffer = (unsigned char*)malloc(len+1);
    unsigned char* out_buff = (unsigned char*)malloc(1024);
    fread(buffer, sizeof(char), len, f);
    fclose(f);
    /* printf("first chars:");
    for(int i = 0; i < 10; i++)
    {
        printf(" 0x%02x", buffer[i]);
    }
    printf("\n"); */
    unsigned char* good_buffer = NULL;
    unsigned char* image_data = NULL;
    //printf("data: %d\n", buffer[0]);
    if(buffer[0] == 1)
    {
        // Decompress using lzss heatshrink lib
        good_buffer = buffer + 4;
        int good_len = len - 4;
        /* printf("first chars:");
        for(int i = 0; i < 10; i++)
        {
            printf(" 0x%02x", good_buffer[i]);
        }
        printf("\n"); */
        //printf("good_len: %d\n", good_len);

        // Decode bm file
        heatshrink_decoder* decoder = heatshrink_decoder_alloc(good_len, 8, 4);
        size_t input_size;
        HSD_sink_res sink_res = heatshrink_decoder_sink(decoder, (uint8_t *)good_buffer, (size_t)good_len, &input_size);
        if(sink_res) {}
        //printf("size consumed: %ld\n", input_size);

        HSD_poll_res poll_res;
        /* while(input_size != 0)
        { */
        poll_res = heatshrink_decoder_poll(decoder, (uint8_t *)out_buff, 1024, &input_size);
        if(poll_res) {}
        heatshrink_decoder_finish(decoder);
        heatshrink_decoder_free(decoder);
        //printf("size polled: %ld\n", input_size);
        /* } */
        /* printf("first chars:");
        for(int i = 0; i < 100; i++)
        {
            printf(" 0x%02x,", out_buff[i]);
        }
        printf("\n"); */

        image_data = (unsigned char*)malloc(1024*8*4);
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

        /* printf("debug this vjsernbgoiuer image_data:\n");
        for(int i = 0; i < 4096; i++)
        {
            printf(", %d", image_data[i]);
        }
        printf("\n"); */

        // what was I trying to do making a png from scratch??
        /* f = stbi__fopen("./tiny.png", "wb");
        if (!f) printf("Failed opening file\n");
        fprintf(f, "\x89PNG\r\n\x1a\n");
        fprintf(f, "%c%c%c\x0D", 0, 0, 0);
        fprintf(f, "IHDR");
        fprintf(f, "%c%c%c\x03", 0, 0, 0);
        fprintf(f, "%c%c%c\x01", 0, 0, 0);
        fprintf(f, "\x08");
        fprintf(f, "\x02");
        fprintf(f, "%c%c%c", 0, 0, 0);
        fprintf(f, "\x94\x82\x83\xE3");
        fprintf(f, "%c%c%c\x15", 0, 0, 0);
        fprintf(f, "IDAT");
        fprintf(f, "\x08\x1D\x01");
        fprintf(f, "\x0A%c", 0);
        fprintf(f, "\xF5\xFF");
        fprintf(f, "%c", 0);
        fprintf(f, "\xFF%c%c%c\xFF%c%c%c\xFF", 0, 0, 0, 0, 0, 0);
        fprintf(f, "\x0E\xFB\x02\xFE");
        fprintf(f, "\xE9\x32\x61\xE5");
        fprintf(f, "%c%c%c%c", 0, 0, 0, 0);
        fprintf(f, "IEND");
        fprintf(f, "\xAE\x42\x60\x82");
        fclose(f); */

    }
    else if(buffer[0] == 0)
    {

    }
    else
        // Houston??
        return -1;

    free(buffer);
    free(out_buff);

    // Load from buffer
    int image_width = 128;
    int image_height = 64;
    /* unsigned char* image_data = stbi_load_from_memory(out_buff, len, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false; */

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
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}

// Main code
int main(int argc, char* argv[])
{
    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to the latest version of SDL is recommended!)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Setup window
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | /* SDL_WINDOW_RESIZABLE |  */SDL_WINDOW_ALLOW_HIGHDPI/*  | SDL_WINDOW_BORDERLESS */);
    SDL_Window* window = SDL_CreateWindow("Flipper Animation Manager", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    int my_image_width = 128;
    int my_image_height = 64;
    //GLuint my_image_texture = 0;
    //bool ret = LoadTextureFromFile("./ooggle.png", &my_image_texture, &my_image_width, &my_image_height);
    //IM_ASSERT(ret);

    AnimationWallet* animations_wallet = new AnimationWallet();
    animations_wallet->add_animation(std::string("../dolphin/L1_Mods_128x64/"));
    animations_wallet->add_animation(std::string("../dolphin/L1_New_year_128x64/"));
    animations_wallet->add_animation(std::string("../dolphin/L1_JILL_HOME_128x64/"));
    animations_wallet->add_animation(std::string("../dolphin/Haseo_A_New_Hope_128x64/"));


    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    io.IniFilename = NULL;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL2_Init();

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    bool show_demo_window = false;
    ImVec4 clear_color = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Toolbox");

            ImGui::Checkbox("Assets window", &show_demo_window);
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
            ImGui::ColorEdit3("clear color", (float*)&clear_color); 

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        ImGui::SetNextWindowPos({0, 690});
        ImGui::SetNextWindowSize({400, 30});
        ImGui::Begin("Framerate Window",
                    NULL,
                    ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoSavedSettings |
                    ImGuiWindowFlags_NoCollapse |
                    ImGuiWindowFlags_NoMove |
                    ImGuiWindowFlags_NoTitleBar
                    );
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    
        ImGui::SetNextWindowPos({0, 0});
        ImGui::SetNextWindowSize({400, 690});
        ImGui::Begin("Left Window",
                    NULL,
                    ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoSavedSettings |
                    ImGuiWindowFlags_NoCollapse |
                    ImGuiWindowFlags_NoMove |
                    ImGuiWindowFlags_NoTitleBar |
                    ImGuiWindowFlags_MenuBar
                    );
        if(ImGui::BeginMenuBar())
        {
            if(ImGui::BeginMenu("Menu"))
            {
                ImGui::MenuItem("(Nothing here for now...)", NULL, false, false);
                ImGui::MenuItem("Quit", "Alt+F4", &done);
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        ImGui::Text("Config File");
        static char configFileText[1024 * 16] = "";
        ImGui::InputTextMultiline("##source", configFileText, IM_ARRAYSIZE(configFileText), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 47), ImGuiInputTextFlags_AllowTabInput);
        ImGui::Button("Save Config File");
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::Text("Not working yet..");
            ImGui::EndTooltip();
        }
        ImGui::End();

        ImGui::SetNextWindowPos({400, 0});
        ImGui::SetNextWindowSize({880, 720});
        ImGui::Begin("Flipper Animation Gallery",
                    NULL,
                    ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoSavedSettings |
                    ImGuiWindowFlags_NoCollapse |
                    ImGuiWindowFlags_NoMove |
                    ImGuiWindowFlags_NoTitleBar
                    );
        /* ImGui::Text("pointer = %d", my_animation[frame_num]);
        ImGui::Text("size = %d x %d", my_image_width, my_image_height); */
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20.f);
        if (ImGui::BeginTable("tableGallery", 2, ImGuiTableFlags_PadOuterX))
        {
            int current_anim = 0;
            while(current_anim < animations_wallet->animations_number)
            {
                if(current_anim % 2 == 0)
                    ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(current_anim % 2);
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.f);
                ImGui::BeginGroup();
                ImGui::PushID(current_anim);
                ImGui::Text("%s", animations_wallet->animations.at(current_anim)->anim_name.c_str());
                ImGui::Image((void*)(intptr_t)animations_wallet->animations.at(current_anim)->get_frame(), ImVec2(my_image_width*3.f, my_image_height*3.f));
                if(ImGui::IsItemClicked())
                {
                    animations_wallet->animations.at(current_anim)->selected^= 1;
                }
                ImGui::Checkbox("Use This Animation", &animations_wallet->animations.at(current_anim)->selected);
                if(animations_wallet->animations.at(current_anim)->selected)
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(255, 255, 255, 20));
                ImGui::PopID();
                ImGui::EndGroup();
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20.f);
                current_anim+= 1;
            }
            /* for (int row = 0; row < 4; row++)
            {
                ImGui::TableNextRow();
                for (int column = 0; column < 2; column++)
                {
                    ImGui::TableSetColumnIndex(column);
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.f);
                    ImGui::BeginGroup();
                    ImGui::Text("%s", anim1->anim_name.c_str());
                    ImGui::Image((void*)(intptr_t)anim1->get_frame(), ImVec2(my_image_width*3.f, my_image_height*3.f));
                    static bool check = false;
                    ImGui::Checkbox("Use This Animation", &check);
                    ImGui::EndGroup();
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20.f);
                }
            } */
            ImGui::EndTable();
        }
        ImGui::End();

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    delete(animations_wallet);

    // Cleanup Imgui
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    // Cleanup backend
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
