#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

extern Display *dpy;
extern Window bar_win;
extern int sw, sh;
extern int current_tag;
extern Client *sel;
extern unsigned long getcolor(const char *colstr);

void drawbar(void) {
    XClearWindow(dpy, bar_win);
    
    char left_text[512];
    char active_title[256] = "";
    if (sel) {
        char *name;
        if (XFetchName(dpy, sel->win, &name)) {
            snprintf(active_title, sizeof(active_title), "%s", name);
            XFree(name);
        }
    }
    snprintf(left_text, sizeof(left_text), "[%s] %s", tags[current_tag], active_title);

    char kernel[32], datetime[64], battery[16], volume[16], brightness[16], cpu[16], ram[16];
    extern void get_kernel(char*, size_t);
    extern void get_datetime(char*, size_t);
    extern void get_battery(char*, size_t);
    extern void get_volume(char*, size_t);
    extern void get_brightness(char*, size_t);
    extern void get_cpu(char*, size_t);
    extern void get_ram(char*, size_t);

    get_kernel(kernel, sizeof(kernel));
    get_datetime(datetime, sizeof(datetime));
    get_battery(battery, sizeof(battery));
    get_volume(volume, sizeof(volume));
    get_brightness(brightness, sizeof(brightness));
    get_cpu(cpu, sizeof(cpu));
    get_ram(ram, sizeof(ram));

    char right_text[1024];
    snprintf(right_text, sizeof(right_text), "%s | %s | %s | %s | %s | %s | %s",
             kernel, datetime, battery, volume, brightness, cpu, ram);

    char corner_text[] = "nyxwm-1.0";

    GC gc = XCreateGC(dpy, bar_win, 0, NULL);
    XFontStruct *font = XLoadQueryFont(dpy, FONT_NAME);
    if (font) {
        XSetFont(dpy, gc, font->fid);
    }

    XSetForeground(dpy, gc, getcolor(FG_COLOR));
    XDrawString(dpy, bar_win, gc, 12, (BAR_HEIGHT / 2) + 4, left_text, strlen(left_text));
    
    int right_width = font ? XTextWidth(font, right_text, strlen(right_text)) : 400;
    int corner_width = font ? XTextWidth(font, corner_text, strlen(corner_text)) : 70;

    XDrawString(dpy, bar_win, gc, sw - (BAR_PADDING_X * 2) - right_width - corner_width - 30, (BAR_HEIGHT / 2) + 4, right_text, strlen(right_text));
    XDrawString(dpy, bar_win, gc, sw - (BAR_PADDING_X * 2) - corner_width - 10, (BAR_HEIGHT / 2) + 4, corner_text, strlen(corner_text));

    if (font) {
        XFreeFont(dpy, font);
    }
    XFreeGC(dpy, gc);
    XFlush(dpy);
}