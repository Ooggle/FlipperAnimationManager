#include "Notifications.hpp"

Notifications::Notifications() {}

Notifications::~Notifications() {}

void Notifications::add_notification(notification_type type, std::string title, std::string comment)
{
    notification notif;
    notif.date_created = std::chrono::system_clock::now();
    notif.type = type;
    notif.title = title,
    notif.comment = comment;
    this->notifications.push_back(notif);
}

void Notifications::display_notifications(int window_width, int window_height)
{
    // Windows style
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(43.f / 255.f, 43.f / 255.f, 43.f / 255.f, 230.f / 255.f));
    int notif_offset = 40;
    int notif_number = 0;

    for(notification notif: this->notifications)
    {
        // if expired
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - notif.date_created;
        if(elapsed_seconds.count() > NOTIFICATIONS_DURATION)
        {
            this->notifications.erase(this->notifications.begin() + notif_number);
        }
        else
        {
            ImGui::SetNextWindowPos(ImVec2(window_width - 40, window_height - notif_offset), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
            ImGui::SetNextWindowSize(ImVec2(300, 70));
            std::string notif_id = std::string("notif##") + std::to_string(notif_number);
            ImGui::Begin(notif_id.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav  | ImGuiWindowFlags_NoFocusOnAppearing);
            switch (notif.type)
            {
            case NOTIF_SUCCESS:
                ImGui::TextColored({0, 255, 0, 255}, ICON_FA_CIRCLE_CHECK);
                break;
            case NOTIF_INFO:
                ImGui::TextColored({0, 157, 255, 255}, ICON_FA_CIRCLE_INFO);
                break;
            case NOTIF_ERROR:
                ImGui::TextColored({255, 0, 0, 255}, ICON_FA_CIRCLE_XMARK);
                break;

            default:
                break;
            }
            
            ImGui::SameLine();
            ImGui::Text("%s", notif.title.c_str());
            ImGui::Separator();
            ImGui::TextWrapped("%s", notif.comment.c_str());
            ImGui::End();

            notif_offset+= 90;
            notif_number+= 1;
        }
    }
    ImGui::PopStyleVar(1);
    ImGui::PopStyleColor(1);
}
