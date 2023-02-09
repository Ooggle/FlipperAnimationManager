#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H

#include <chrono>
#include <string>
#include <vector>
#include "imgui.h"

// fonts
#include "../fonts/IconsFontAwesome6.h"
#include "../fonts/IconsFontAwesome6Brands.h"

#define NOTIFICATIONS_DURATION 3 // 3sec

typedef enum 
{ 
    NOTIF_SUCCESS,
    NOTIF_INFO,
    NOTIF_ERROR
} notification_type;

struct notification
{
    std::chrono::system_clock::time_point date_created;
    notification_type type;
    std::string title;
    std::string comment;
};

class Notifications
{
    private:
        std::vector<notification> notifications;

    public:
        Notifications();
        ~Notifications();
        void add_notification(notification_type type, std::string title, std::string comment);
        void display_notifications(int window_width, int window_height);
};

#endif
