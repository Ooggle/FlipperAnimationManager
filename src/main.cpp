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

// Main code
int main(int argc, char* argv[])
{
    const int version_major = 0;
    const int version_minor = 9;
    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to the latest version of SDL is recommended!)
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
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

    char current_animations_folder[1024] = "./dolphin/";

    AnimationWallet* animations_wallet = new AnimationWallet(current_animations_folder);

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

    // animations size:
    const int image_width = 128;
    const int image_height = 64;

    // Our state
    bool show_toolbox = false;
    bool show_demo_window = false;
    ImVec4 clear_color = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);

    // Main loop
    bool done = false;
    bool first_cycle = true;
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
            if(event.type == SDL_QUIT)
                done = true;
            if(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

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
                ImGui::MenuItem("(...)", NULL, false, false);
                ImGui::MenuItem("About");
                if(ImGui::IsItemClicked())
                    ImGui::OpenPopup("About");
                
                if(ImGui::BeginPopupModal("About", NULL, ImGuiWindowFlags_NoResize))
                {
                    ImGui::Text("Flipper-Zero Animation Manager v%d.%d", version_major, version_minor);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.f);
                    ImGui::Text("Made with <3 by Ooggle");
                    ImGui::Text("https://github.com/Ooggle"); // TODO: clickable link
                    ImGui::Separator();
                    if(ImGui::Button("Close") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
                        ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                }
                ImGui::MenuItem("Quit", "Alt+F4", &done);
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        ImGui::InputText("Dolphin folder", current_animations_folder, 1024);
        if(ImGui::Button("Load"))
        {
            delete(animations_wallet);
            animations_wallet = new AnimationWallet(current_animations_folder);
        }

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.f);
        ImGui::Separator();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.f);
        ImGui::Text("Manifest File (work in progress)");
        static char configFileText[1024 * 16] = {0};
        ImGui::InputTextMultiline("##source", configFileText, IM_ARRAYSIZE(configFileText), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 20), ImGuiInputTextFlags_ReadOnly);
        if(ImGui::Button("Copy Manifest to clipboard"))
        {
            ImGui::SetClipboardText(configFileText);
            // TODO: show copied to clipboard message
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
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20.f);
        if(animations_wallet->animations_number != 0)
        {
            if(ImGui::BeginTable("tableGallery", 2, ImGuiTableFlags_PadOuterX))
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
                    ImGui::Image((void*)(intptr_t)animations_wallet->animations.at(current_anim)->get_frame(), ImVec2(image_width*3.f, image_height*3.f));
                    if(ImGui::IsItemClicked())
                    {
                        animations_wallet->animations.at(current_anim)->selected^= 1;
                    }
                    ImGui::Checkbox("Use This Animation", &animations_wallet->animations.at(current_anim)->selected);
                    ImGui::SameLine();
                    if(ImGui::Button("Preview"))
                        ImGui::OpenPopup("Fullscreen Preview");
                    ImGui::SameLine();
                    if(ImGui::Button("Reload"))
                        animations_wallet->animations.at(current_anim)->reload_animation();
                    // TODO: add weight

                    if(ImGui::BeginPopupModal("Fullscreen Preview", NULL, ImGuiWindowFlags_NoResize))
                    {
                        ImGui::Image((void*)(intptr_t)animations_wallet->animations.at(current_anim)->get_frame(), ImVec2(image_width*7.f, image_height*7.f));
                        if(ImGui::Button("Close") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
                            ImGui::CloseCurrentPopup();
                        ImGui::SameLine();
                        if(ImGui::Button("Reload animation"))
                            animations_wallet->animations.at(current_anim)->reload_animation();
                        ImGui::SameLine();
                        ImGui::Text("Frame %d/%d", animations_wallet->animations.at(current_anim)->get_current_frame_number() + 1, animations_wallet->animations.at(current_anim)->get_total_frames_files());
                        ImGui::EndPopup();
                    }
                    if(animations_wallet->animations.at(current_anim)->selected)
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(255, 255, 255, 20));
                    ImGui::PopID();
                    ImGui::EndGroup();
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20.f);
                    current_anim+= 1;
                }
                ImGui::EndTable();
            }
        }
        else
        {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 300.f);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 175.f);
            ImGui::SetWindowFontScale(2.f);
            ImGui::Text("No animation found in selected folder.");
            ImGui::SetWindowFontScale(1.f);
        }
        ImGui::End();

        if(show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);


        if(show_toolbox)
        {
            //static float f = 0.0f;
            if(first_cycle)
            {
                ImGui::SetNextWindowPos({30, 550});
                ImGui::SetNextWindowCollapsed(true, 2);
            }
            ImGui::Begin("Toolbox");
            ImGui::Checkbox("Assets", &show_demo_window);
            /* ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
            ImGui::ColorEdit3("clear color", (float*)&clear_color); */

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
        if(first_cycle)
            first_cycle = false;
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
