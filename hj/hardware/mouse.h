#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>

#ifdef __linux__
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <dirent.h>

#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>

#elif __APPLE__
#include <ApplicationServices/ApplicationServices.h>

#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    char device_path[256];
    char manufacturer[128];
    char product[128];
    char serial[128];
} mouse_info_t;

typedef struct
{
    int x;
    int y;
    int button;
    int pressed;
    int wheel;
} mouse_event_t;

inline int mouse_enumerate(mouse_info_t *infos, int max_count)
{
    int count = 0;

#ifdef __linux__
    DIR *dir = opendir("/dev/input");
    if(!dir)
        return 0;

    struct dirent *entry;
    while((entry = readdir(dir)) && count < max_count)
    {
        if(strncmp(entry->d_name, "event", 5) != 0)
            continue;

        char path[256];
        snprintf(path, sizeof(path), "/dev/input/%s", entry->d_name);
        int fd = open(path, O_RDONLY);
        if(fd < 0)
            continue;

        unsigned long evbit = 0;
        ioctl(fd, EVIOCGBIT(0, sizeof(evbit)), &evbit);
        if(evbit & (1 << EV_REL))
        {
            struct input_id id;
            ioctl(fd, EVIOCGID, &id);
            strncpy(infos[count].device_path,
                    path,
                    sizeof(infos[count].device_path));
            snprintf(infos[count].manufacturer,
                     sizeof(infos[count].manufacturer),
                     "vendor_%04x",
                     id.vendor);
            snprintf(infos[count].product,
                     sizeof(infos[count].product),
                     "product_%04x",
                     id.product);
            snprintf(infos[count].serial,
                     sizeof(infos[count].serial),
                     "version_%04x",
                     id.version);
            close(fd);
            count++;
        } else
        {
            close(fd);
        }
    }
    closedir(dir);

#elif defined(_WIN32) || defined(_WIN64)
    strncpy(infos[0].device_path, "Win32Mouse", sizeof(infos[0].device_path));
    strncpy(infos[0].manufacturer, "Microsoft", sizeof(infos[0].manufacturer));
    strncpy(infos[0].product, "Generic Mouse", sizeof(infos[0].product));
    infos[0].serial[0] = 0;
    count              = 1;

#elif defined(__APPLE__)
    strncpy(infos[0].device_path, "MacMouse", sizeof(infos[0].device_path));
    strncpy(infos[0].manufacturer, "Apple", sizeof(infos[0].manufacturer));
    strncpy(infos[0].product, "Generic Mouse", sizeof(infos[0].product));
    infos[0].serial[0] = 0;
    count              = 1;

#endif

    return count;
}

inline int mouse_open(const char *device_path)
{
#ifdef __linux__
    return open(device_path, O_RDONLY | O_NONBLOCK);

#elif defined(_WIN32) || defined(__APPLE__)
    (void) device_path;
    return 1;

#endif
}

inline void mouse_close(int handle)
{
#ifdef __linux__
    if(handle >= 0)
        close(handle);

#else
    (void) handle;

#endif
}

inline int mouse_read_event(int handle, mouse_event_t *event)
{
#ifdef __linux__
    struct input_event ev;
    ssize_t            n = read(handle, &ev, sizeof(ev));
    if(n == sizeof(ev))
    {
        if(ev.type == EV_REL)
        {
            if(ev.code == REL_X)
                event->x += ev.value;

            if(ev.code == REL_Y)
                event->y += ev.value;

            if(ev.code == REL_WHEEL)
                event->wheel += ev.value;
        }
        if(ev.type == EV_KEY && (ev.code >= BTN_LEFT && ev.code <= BTN_MIDDLE))
        {
            event->button  = ev.code - BTN_LEFT + 1;
            event->pressed = ev.value;
            return 0;
        }
    }
    return -1;

#elif defined(_WIN32) || defined(_WIN64)
    POINT pt;
    GetCursorPos(&pt);
    event->x       = pt.x;
    event->y       = pt.y;
    event->wheel   = 0;
    event->button  = 0;
    event->pressed = 0;
    if(GetAsyncKeyState(VK_LBUTTON) & 0x8000)
    {
        event->button  = 1;
        event->pressed = 1;
    }
    if(GetAsyncKeyState(VK_RBUTTON) & 0x8000)
    {
        event->button  = 2;
        event->pressed = 1;
    }
    if(GetAsyncKeyState(VK_MBUTTON) & 0x8000)
    {
        event->button  = 3;
        event->pressed = 1;
    }
    return 0;

#elif defined(__APPLE__)
    CGEventRef eventRef = CGEventCreate(NULL);
    CGPoint    pt       = CGEventGetLocation(eventRef);
    event->x            = (int) pt.x;
    event->y            = (int) pt.y;
    event->wheel        = 0;
    event->button       = 0;
    event->pressed      = 0;
    CFRelease(eventRef);
    return 0;

#endif
}

inline int mouse_set_param(int handle, int accel)
{
#ifdef __linux__
    // TODO
    return 0;

#elif defined(_WIN32)
    (void) handle;
    return SystemParametersInfoA(SPI_SETMOUSESPEED,
                                 0,
                                 (PVOID) (intptr_t) accel,
                                 0)
               ? 0
               : -1;

#elif defined(__APPLE__)
    // TODO
    return 0;

#endif
}

#ifdef __cplusplus
}
#endif

#endif // MOUSE_H