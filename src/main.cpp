#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl2.h"
#include "file_browser/ImGuiFileBrowser.h"
#include "Animation.hpp"
#include "AnimationWallet.hpp"
#include "Manifest.hpp"
#include "utils/Notifications.hpp"
#include <stdio.h>
#include <SDL.h>
#include <SDL_opengl.h>

// fonts
#include "fonts/born_2_b_sporty.h"
#include "fonts/fa_solid_900.h"
#include "fonts/fa_brands_400.h"
#include "fonts/IconsFontAwesome6.h"
#include "fonts/IconsFontAwesome6Brands.h"

// for system()
#include <stdlib.h>

#ifdef _WIN32
#include <shellapi.h>
#endif

extern "C" {
    #include "heatshrink_decoder.h"
}

/* Helper from ImGui demo */
static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 30.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

// Main code
int main(int argc, char* argv[])
{
    const int version_major = 1;
    const int version_minor = 3;
    const int version_patch = 1;
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
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI/*  | SDL_WINDOW_BORDERLESS */);
    SDL_Window* window = SDL_CreateWindow("Flipper Animation Manager", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_SetWindowMinimumSize(window, 1280, 720);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    AnimationWallet* animations_wallet;

    char current_animations_folder[1024] = {0};
    if(argc > 1)
    {
        if(strlen(argv[1]) <= 1024)
        {
            strcpy(current_animations_folder, argv[1]);
            animations_wallet = new AnimationWallet(current_animations_folder);
        }
        else
            animations_wallet = new AnimationWallet();
    }
    else
        animations_wallet = new AnimationWallet();

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
    //ImFont* font = io.Fonts->AddFontFromFileTTF("./Born2bSportyV2.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);
    ImFontConfig font_cfg;
    font_cfg.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontDefault();
    static const ImWchar icons_ranges_fa[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    static const ImWchar icons_ranges_fab[] = {ICON_MIN_FAB, ICON_MAX_FAB, 0};
    font_cfg.MergeMode = true;
    font_cfg.PixelSnapH = true;
    io.Fonts->AddFontFromMemoryCompressedTTF((void*)fa_solid_900_compressed_data, fa_solid_900_compressed_size, 12.f, &font_cfg, icons_ranges_fa);
    io.Fonts->AddFontFromMemoryCompressedTTF((void*)fa_brands_400_compressed_data, fa_brands_400_compressed_size, 12.f, &font_cfg, icons_ranges_fab);
    font_cfg.PixelSnapH = false;
    font_cfg.MergeMode = false;
    ImFont* sporty_font = io.Fonts->AddFontFromMemoryCompressedTTF((void*)born_2_b_sporty_compressed_data, born_2_b_sporty_compressed_size, 16.f, &font_cfg);

    imgui_addons::ImGuiFileBrowser file_dialog; // As a class member or globally

    Notifications notifications;

    // animations size:
    const int image_width = 128;
    const int image_height = 64;

    char* manifest_content_char = (char*)malloc(sizeof(char) * 1000);
    long unsigned int manifest_content_max_size = sizeof(char) * 1000;

    // global variables for specific windows
    // weight window
    int old_weight_replace = 7;
    int new_weight_replace = 7;
    int new_weight = 7;
    // level window
    int new_min_level = 0;
    int new_max_level = 30;
    // butthurt window
    int new_min_butthurt = 0;
    int new_max_butthurt = 14;

    // Our state
    int window_width = 1280;
    int window_height = 720;
    bool show_toolbox = false;
    bool show_demo_window = false;
    bool theater_mode = false;
    float default_font_size = 1.2f;
    bool animation_wallet_loaded = false;
    io.FontGlobalScale = 1.2f;
    ImVec4 clear_color = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);

    // Main loop
    bool done = false;
    bool first_cycle = true;
    while(!done)
    {
        std::string manifest_content = "Filetype: Flipper Animation Manifest\nVersion: 1\n";
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if(event.type == SDL_QUIT)
                done = true;
            if(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
            if(event.window.event == SDL_WINDOWEVENT_RESIZED)
            {
                SDL_GetWindowSize(window, &window_width, &window_height);
            }
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // TODO: remove this window by default
        ImGui::SetNextWindowPos({0, 690 + ((float)window_height - 720)});
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

        ImGui::SetNextWindowPos({400, 0});
        ImGui::SetNextWindowSize({880 + ((float)window_width - 1280), 720 + ((float)window_height - 720)});
        ImGui::Begin("Flipper Animation Gallery",
                    NULL,
                    ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoSavedSettings |
                    ImGuiWindowFlags_NoCollapse |
                    ImGuiWindowFlags_NoMove |
                    ImGuiWindowFlags_NoTitleBar |
                    ImGuiWindowFlags_NoBringToFrontOnFocus
                    );
        if(animations_wallet->animations_number != 0 && !animations_wallet->not_initialized)
        {
            if(!theater_mode)
            {
                if(ImGui::Button("Select all"))
                {
                    int current_anim = 0;
                    while(current_anim < animations_wallet->animations_number)
                    {
                        animations_wallet->animations.at(current_anim)->selected = true;
                        current_anim+= 1;
                    }
                }
                ImGui::SameLine();
                if(ImGui::Button("Deselect all"))
                {
                    int current_anim = 0;
                    while(current_anim < animations_wallet->animations_number)
                    {
                        animations_wallet->animations.at(current_anim)->selected = false;
                        current_anim+= 1;
                    }
                }
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20.f);
            }

            int row_num = 1 + ((window_width - 850) / 420);

            if(ImGui::BeginTable("tableGallery", row_num, ImGuiTableFlags_PadOuterX))
            {
                int current_anim = 0;
                while(current_anim < animations_wallet->animations_number)
                {
                    if(current_anim % row_num == 0)
                        ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(current_anim % row_num);
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.f);
                    ImGui::BeginGroup();
                    ImGui::PushID(current_anim);
                    ImGui::Text("%s", animations_wallet->animations.at(current_anim)->anim_name.c_str());
                    if(!theater_mode)
                    {
                        ImGui::SameLine();
                        if(animations_wallet->animations.at(current_anim)->format == BM)
                        {
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.25f, 1.f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.25f, 1.f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.25f, 0.25f, 1.f));
                            ImGui::Button("bm");
                            ImGui::PopStyleColor(3);
                            if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
                            {
                                ImGui::BeginTooltip();
                                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                                ImGui::TextUnformatted(".bm files represents compiled animation files that can directly be used by the Flipper Zero.");
                                ImGui::PopTextWrapPos();
                                ImGui::EndTooltip();
                            }
                        }
                        else if(animations_wallet->animations.at(current_anim)->format == PNG)
                        {
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.25f, 1.f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.25f, 1.f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.25f, 0.25f, 1.f));
                            ImGui::Button("png");
                            ImGui::PopStyleColor(3);
                            if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
                            {
                                ImGui::BeginTooltip();
                                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                                // TODO: Better description?
                                ImGui::TextUnformatted(".png files. This is only a preview, this animation needs to be compiled first before being added to the Flipper Zero.");
                                ImGui::PopTextWrapPos();
                                ImGui::EndTooltip();
                            }
                        }
                    }
                    
                    ImGui::Image((void*)(intptr_t)animations_wallet->animations.at(current_anim)->get_frame(), ImVec2(image_width*3.f, image_height*3.f));
                    if(ImGui::IsItemClicked())
                    {
                        animations_wallet->animations.at(current_anim)->selected^= 1;
                    }
                    if(!theater_mode)
                    {
                        ImGui::Checkbox("Use This Animation", &animations_wallet->animations.at(current_anim)->selected);
                        ImGui::SameLine();
                        if(ImGui::Button("Reload"))
                            animations_wallet->animations.at(current_anim)->reload_animation();
                        ImGui::SameLine();
                        if(ImGui::Button("More"))
                            ImGui::OpenPopup(animations_wallet->animations.at(current_anim)->anim_name.c_str());
                        ImGui::SliderInt("Weight", &animations_wallet->animations.at(current_anim)->weight, 0, 14);
                        // TODO: add leveling
                    }

                    // popup modal for animation preview
                    bool not_used_popup_modal_preview = true;
                    bool active_frames = animations_wallet->animations.at(current_anim)->active;
                    if(ImGui::BeginPopupModal(animations_wallet->animations.at(current_anim)->anim_name.c_str(), &not_used_popup_modal_preview, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar))
                    {
                        ImGui::Image((void*)(intptr_t)animations_wallet->animations.at(current_anim)->get_frame(), ImVec2(image_width*7.f, image_height*7.f));
                        if(ImGui::Button("Close") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
                            ImGui::CloseCurrentPopup();
                        ImGui::SameLine();
                        if(ImGui::Button("Reload animation"))
                            animations_wallet->animations.at(current_anim)->reload_animation();
                        ImGui::SameLine();
                        if(animations_wallet->animations.at(current_anim)->format == PNG)
                        {
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{1.f, 0.5f, 0.f, 0.7f});
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{1.f, 0.5f, 0.f, 0.9f});
                            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{1.f, 0.5f, 0.f, 1.f});
                            if(ImGui::Button("Compile animation for Flipper (beta)"))
                            {
                                animations_wallet->animations.at(current_anim)->export_to_bm();
                                ImGui::OpenPopup("exported_to_bm");
                            }
                            ImGui::PopStyleColor(3);
                            //ImGui::SetNextWindowSize({700, 50});
                            if(ImGui::BeginPopupModal("exported_to_bm", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar))
                            {
                                ImGui::SetWindowFontScale(1.5f);
                                ImGui::Text("Exported to folder:");
                                ImGui::Text("%s_compiled/", animations_wallet->animations.at(current_anim)->anim_folder.substr(0, animations_wallet->animations.at(current_anim)->anim_folder.length() - 1).c_str());
                                if(ImGui::Button("Close") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
                                    ImGui::CloseCurrentPopup();
                                ImGui::SetWindowFontScale(default_font_size);
                                ImGui::EndPopup();
                            }
                        }
                        ImGui::SameLine();
                        ImGui::Text("Frame %03d/%03d", animations_wallet->animations.at(current_anim)->get_current_frame_number() + 1, animations_wallet->animations.at(current_anim)->get_total_frames_number());
                        ImGui::SameLine();
                        if(ImGui::Checkbox("Active frames", &active_frames)) {
                            if (active_frames)
                                animations_wallet->animations.at(current_anim)->active = true;
                            else
                                animations_wallet->animations.at(current_anim)->active = false;
                        }
                        ImGui::Separator();
                        // level sliders
                        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.385f);
                        ImGui::SliderInt("Min level", &animations_wallet->animations.at(current_anim)->min_level, 0, animations_wallet->animations.at(current_anim)->max_level);
                        ImGui::SameLine();
                        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.385f);
                        ImGui::SliderInt("Max level", &animations_wallet->animations.at(current_anim)->max_level, animations_wallet->animations.at(current_anim)->min_level, 30);
                        ImGui::Separator();
                        // butthurt sliders
                        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.35f);
                        ImGui::SliderInt("Min butthurt", &animations_wallet->animations.at(current_anim)->min_butthurt, 0, animations_wallet->animations.at(current_anim)->max_butthurt);
                        ImGui::SameLine();
                        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.35f);
                        ImGui::SliderInt("Max butthurt", &animations_wallet->animations.at(current_anim)->max_butthurt, animations_wallet->animations.at(current_anim)->min_butthurt, 14);

                        ImGui::EndPopup();
                    }
                    if(animations_wallet->animations.at(current_anim)->selected)
                    {
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(255, 255, 255, 20));
                        // update manifest content
                        manifest_content.append("\nName: " + animations_wallet->animations.at(current_anim)->anim_name + "\nMin butthurt: " +
                            std::to_string(animations_wallet->animations.at(current_anim)->min_butthurt) + "\nMax butthurt: " +
                            std::to_string(animations_wallet->animations.at(current_anim)->max_butthurt) + "\nMin level: " +
                            std::to_string(animations_wallet->animations.at(current_anim)->min_level) + "\nMax level: " +
                            std::to_string(animations_wallet->animations.at(current_anim)->max_level) + "\nWeight: " +
                            std::to_string(animations_wallet->animations.at(current_anim)->weight) + "\n");
                    }

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
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ((window_height - 100) / 2));
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 70.f + ((window_width - 850) / 2));
            ImGui::SetWindowFontScale(2.f);
            if(animations_wallet->not_initialized)
                ImGui::Text("Please select a folder.");
            else
                ImGui::Text("No animation found in selected folder.");
            ImGui::SetWindowFontScale(default_font_size);
        }
        ImGui::End();

        ImGui::SetNextWindowPos({0, 0});
        ImGui::SetNextWindowSize({400, 690 + ((float)window_height - 720)});
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
                ImGui::Checkbox("Minimal UI Mode", &theater_mode);

                ImGui::MenuItem("About");
                if(ImGui::IsItemClicked())
                    ImGui::OpenPopup("About");
                
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));
                ImGui::SetNextWindowSize(ImVec2(500, 350));
                if(ImGui::BeginPopupModal("About", NULL, ImGuiWindowFlags_NoResize))
                {
                    ImGui::PopStyleVar();

                    if(ImGui::BeginTabBar("about_tab_bar", ImGuiTabBarFlags_None))
                    {
                        if(ImGui::BeginTabItem("About"))
                        {
                            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.f);
                            ImGui::PushFont(sporty_font);
                            ImGui::Text("Flipper-Zero Animation Manager v%d.%d.%d", version_major, version_minor, version_patch);
                            ImGui::PopFont();
                            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.f);
                            ImGui::Text("Made with <3 by Ooggle");
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.3f, 0.3f, 0.3f, 0.2f});
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.7f, 0.4f, 0.1f, 0.3f});
                            ImGui::Text("%s", ICON_FA_GITHUB);
                            ImGui::SameLine();
                            ImGui::Button("https://github.com/Ooggle/FlipperAnimationManager");
                            ImGui::PopStyleColor(2);
                            if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
                            {
                                ImGui::BeginTooltip();
                                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                                ImGui::TextUnformatted("Open link");
                                ImGui::PopTextWrapPos();
                                ImGui::EndTooltip();
                            }
                            if(ImGui::IsItemClicked())
                            {
                                std::string github_link = "https://github.com/Ooggle/FlipperAnimationManager";

                                #ifdef _WIN32
                                ShellExecuteA(NULL, "open", github_link.c_str(), NULL, NULL, SW_SHOWDEFAULT);
                                #else
                                #if __APPLE__
                                std::string command_macos = std::string("open \"") + github_link.c_str() + std::string("\"");
                                system(command_macos.c_str());
                                #else
                                std::string command_linux = std::string("xdg-open \"") + github_link.c_str() + std::string("\"");
                                system(command_linux.c_str());
                                #endif
                                #endif
                            }
                            ImGui::EndTabItem();
                        }
                        if(ImGui::BeginTabItem("Tips"))
                        {
                            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.f);
                            ImGui::PushFont(sporty_font);
                            ImGui::Text("Various tips");
                            ImGui::PopFont();
                            ImGui::TextWrapped("- You can close almost all popups by pressing the Escape key (this one too!)");
                            ImGui::TextWrapped("- Keyboard navigation is possible with 'ctrl + tab' and arrow keys");
                            ImGui::TextWrapped("- It is recommended to run the program on your graphic card for blazing fast performance!");
                            ImGui::TextWrapped("- You can start the program with a pre-choose folder by passing it as command line argument");
                            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.f);
                            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 201, 40, 255));
                            ImGui::TextWrapped("... And thank you for using the Animation Manager!");
                            ImGui::PopStyleColor();
                            ImGui::SameLine();
                            ImGui::TextColored({255, 0, 0, 255}, ICON_FA_HEART);
                            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.f);
                            ImGui::EndTabItem();
                        }
                        ImGui::EndTabBar();
                    }


                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.f);
                    ImGui::Separator();
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.f);
                    if(ImGui::Button("Close") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
                        ImGui::CloseCurrentPopup();
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));
                    ImGui::EndPopup();
                }
                ImGui::PopStyleVar();
                ImGui::MenuItem("Quit", "Alt+F4", &done);
                ImGui::EndMenu();
            }
            if(ImGui::BeginMenu("Tools"))
            {
                ImGui::MenuItem("Change weight...");
                if(ImGui::IsItemClicked())
                    ImGui::OpenPopup("Change weight");

                ImGui::MenuItem("Change level...");
                if(ImGui::IsItemClicked())
                    ImGui::OpenPopup("Change level");

                ImGui::MenuItem("Change butthurt...");
                if(ImGui::IsItemClicked())
                    ImGui::OpenPopup("Change butthurt");
                ImGui::MenuItem("Show only passive frames...");
                if(ImGui::IsItemClicked()) {
                    animations_wallet->set_active(false);
                    notifications.add_notification(NOTIF_SUCCESS, "Success", "Show only passive frames for every animations.");
                }
                ImGui::MenuItem("Show active frames...");
                if(ImGui::IsItemClicked()) {
                    animations_wallet->set_active(true);
                    notifications.add_notification(NOTIF_SUCCESS, "Success", "Show active frames for every animations.");
                }

                bool change_weight_popup = true;
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(30, 20));
                if(ImGui::BeginPopupModal("Change weight", &change_weight_popup, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar))
                {
                    ImGui::PopStyleVar();

                    if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
                        ImGui::CloseCurrentPopup();

                    ImGui::PushFont(sporty_font);
                    ImGui::Text("CHANGE FOR EVERY ANIMATIONS");
                    ImGui::PopFont();
                    ImGui::Text("New weight:");
                    ImGui::SliderInt("##new_weight", &new_weight, 0, 14);
                    if(ImGui::Button("Apply changes##apply_changes"))
                    {
                        animations_wallet->replace_weight(new_weight);
                        notifications.add_notification(NOTIF_SUCCESS, "Success", "Weight has been changed for every animations.");
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.f);
                    ImGui::Separator();
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.f);
                    ImGui::PushFont(sporty_font);
                    ImGui::Text("REPLACE SPECIFIC WEIGHT");
                    ImGui::PopFont();
                    ImGui::Text("Replace:");
                    ImGui::SliderInt("##old_weight_replace", &old_weight_replace, 0, 14);
                    ImGui::Text("By:");
                    ImGui::SliderInt("##new_weight_replace", &new_weight_replace, 0, 14);
                    if(ImGui::Button("Apply changes##apply_changes_replace"))
                    {
                        animations_wallet->replace_weight(new_weight_replace, old_weight_replace);
                        notifications.add_notification(NOTIF_SUCCESS, "Success", "Weight has been replaced for selected animations.");
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));
                    ImGui::EndPopup();
                }
                ImGui::PopStyleVar();

                bool change_level_popup = true;
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(30, 20));
                if(ImGui::BeginPopupModal("Change level", &change_level_popup, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar))
                {
                    ImGui::PopStyleVar();

                    if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
                        ImGui::CloseCurrentPopup();

                    ImGui::PushFont(sporty_font);
                    ImGui::Text("CHANGE FOR EVERY ANIMATIONS");
                    ImGui::PopFont();
                    ImGui::Text("New min level:");
                    ImGui::SliderInt("##new_min_level", &new_min_level, 0, 30);
                    ImGui::Text("New max level:");
                    ImGui::SliderInt("##new_max_level", &new_max_level, 0, 30);
                    if(ImGui::Button("Apply changes##apply_changes"))
                    {
                        if(new_min_level > new_max_level)
                        {
                            notifications.add_notification(NOTIF_ERROR, "Error", "Min level can't be greater than max level!");
                            ImGui::CloseCurrentPopup();
                        }
                        else
                        {
                            animations_wallet->replace_min_max_level(new_min_level, new_max_level);
                            notifications.add_notification(NOTIF_SUCCESS, "Success", "Min and max level has been changed for every animations.");
                            ImGui::CloseCurrentPopup();
                        }
                    }

                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));
                    ImGui::EndPopup();
                }
                ImGui::PopStyleVar();

                bool change_butthurt_popup = true;
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(30, 20));
                if(ImGui::BeginPopupModal("Change butthurt", &change_butthurt_popup, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar))
                {
                    ImGui::PopStyleVar();

                    if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
                        ImGui::CloseCurrentPopup();

                    ImGui::PushFont(sporty_font);
                    ImGui::Text("CHANGE FOR EVERY ANIMATIONS");
                    ImGui::PopFont();
                    ImGui::Text("New min butthurt:");
                    ImGui::SliderInt("##new_min_butthurt", &new_min_butthurt, 0, 14);
                    ImGui::Text("New max butthurt:");
                    ImGui::SliderInt("##new_max_butthurt", &new_max_butthurt, 0, 14);
                    if(ImGui::Button("Apply changes##apply_changes"))
                    {
                        if(new_min_butthurt > new_max_butthurt)
                        {
                            notifications.add_notification(NOTIF_ERROR, "Error", "Min butthurt can't be greater than max butthurt!");
                            ImGui::CloseCurrentPopup();
                        }
                        else
                        {
                            animations_wallet->replace_min_max_butthurt(new_min_butthurt, new_max_butthurt);
                            notifications.add_notification(NOTIF_SUCCESS, "Success", "Min and max butthurt has been changed for every animations.");
                            ImGui::CloseCurrentPopup();
                        }
                    }

                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));
                    ImGui::EndPopup();
                }
                ImGui::PopStyleVar();

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        ImGui::PushFont(sporty_font);
        ImGui::Text("Dolphin folder");
        ImGui::PopFont();
        if(ImGui::InputText("##dolphin_folder", current_animations_folder, 1024, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            animation_wallet_loaded = false;
            delete(animations_wallet);
            animations_wallet = new AnimationWallet(current_animations_folder);
        }
        if(!animation_wallet_loaded)
        {
            if(!animations_wallet->is_finished_loading())
                animations_wallet->load_new_animation();
            else
            {
                animation_wallet_loaded = true;
                animations_wallet->parse_manifest();
            }
        }
        ImGui::SameLine();
        if(ImGui::Button("Select"))
            ImGui::OpenPopup("Select Folder");
        
        if(!animation_wallet_loaded)
        {
            ImGui::OpenPopup("Loading animations popup");
        }
        ImGui::SetNextWindowSize({700, 70});
        if(ImGui::BeginPopupModal("Loading animations popup", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar))
        {
            if(animation_wallet_loaded)
                ImGui::CloseCurrentPopup();
            ImGui::SetWindowFontScale(1.5f);
            std::string load_text = "Loading " + std::to_string(animations_wallet->total_animations__loading) + " animations... ";

            float windowWidth = ImGui::GetWindowSize().x;
            float textWidth = ImGui::CalcTextSize(load_text.c_str()).x;

            ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
            ImGui::Text("%s", load_text.c_str());
            char buf[32];
            sprintf(buf, "%d/%d", animations_wallet->animations_number, animations_wallet->total_animations__loading);
            ImGui::ProgressBar((float)animations_wallet->animations_number / animations_wallet->total_animations__loading, ImVec2(-FLT_MIN, 0.f), buf);

            ImGui::SetWindowFontScale(default_font_size);
            ImGui::EndPopup();
        }
            
        if(file_dialog.showFileDialog("Select Folder", imgui_addons::ImGuiFileBrowser::DialogMode::SELECT, ImVec2(700, 510)))
        {
            animation_wallet_loaded = false;
            strcpy(current_animations_folder, file_dialog.selected_path.c_str());
            delete(animations_wallet);
            animations_wallet = new AnimationWallet(current_animations_folder);
        }

        if(ImGui::Button("Load"))
        {
            animation_wallet_loaded = false;
            delete(animations_wallet);
            animations_wallet = new AnimationWallet(current_animations_folder);
        }
        if(animations_wallet->get_is_folder_correct())
        {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.00f), "Success, %d animations loaded", animations_wallet->animations_number);
        }
        else if(!animations_wallet->not_initialized)
        {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.00f), "Failed, folder not found");
            ImGui::SameLine();
            HelpMarker("The path can be either a relative path from the program location or a absolute (full) path to the dolphin folder which contains compiled animations (with .bm or .png files)\n\nExample of paths:\nWindows: C:\\Users\\user\\dolphin\\\nLinux/MacOS: /path/to/dolphin/folder/");
        }

        // show animations errors if necessary
        if(animations_wallet->errored_animations.size() > 0 && !animations_wallet->not_initialized)
        {
            std::string btn_errors_string = std::to_string(animations_wallet->errored_animations.size()) + ((animations_wallet->errored_animations.size() == 1) ? std::string(" error") : std::string(" errors!"));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.3f, 0.4f, 1.0f});
            if(ImGui::Button(btn_errors_string.c_str()))
                ImGui::OpenPopup("Errored animations");
            ImGui::PopStyleColor(1);

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));
            bool open_popup = true;
            if(ImGui::BeginPopupModal("Errored animations", &open_popup, ImGuiWindowFlags_NoResize))
            {
                ImGui::Text("The following animations did not load due to error:");
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.f);
                for(std::string anim_error: animations_wallet->errored_animations)
                {
                    ImGui::Text("%s", anim_error.c_str());
                }
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.f);
                ImGui::Separator();
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.f);
                if(ImGui::Button("Close") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
                    ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
            }
            ImGui::PopStyleVar();
        }

        if(!theater_mode)
        {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.f);
            ImGui::Separator();
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.f);

            ImGui::PushFont(sporty_font);
            ImGui::Text("Manifest.txt");
            ImGui::PopFont();
            if(strcmp(manifest_content_char, manifest_content.c_str()))
            {
                // Not very pretty but it will remain like that until I found something better without abusing (re)allocations
                while(sizeof(char) * manifest_content.size() > manifest_content_max_size)
                {
                    manifest_content_max_size+= sizeof(char) * 1000;
                    manifest_content_char = (char*)realloc(manifest_content_char, manifest_content_max_size);
                }
                strcpy(manifest_content_char, manifest_content.c_str());
            }
            ImGui::InputTextMultiline("##source", manifest_content_char, manifest_content.size(), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 40), ImGuiInputTextFlags_ReadOnly);
            if(ImGui::Button("Save manifest.txt"))
            {
                if(animations_wallet->update_manifest(manifest_content))
                {
                    notifications.add_notification(NOTIF_SUCCESS, "Success", "manifest.txt successfully updated.");
                }
                else
                {
                    notifications.add_notification(NOTIF_ERROR, "Error", "Error while updating manifest.txt.");
                }
            }
            ImGui::SameLine();
            if(ImGui::Button("Copy to clipboard"))
            {
                ImGui::SetClipboardText(manifest_content_char);
                notifications.add_notification(NOTIF_SUCCESS, "Success", "Manifest copied to clipboard.");
            }
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

        notifications.display_notifications(window_width, window_height);

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
    free(manifest_content_char);

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
