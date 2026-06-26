#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include "config.h"

Display *dpy;
Window root;
Window bar_win;
Window wm_check_win;
int sw, sh;

extern int running;
extern void handle_event(XEvent *ev);
extern void drawbar(void);

static int xerror(Display *d, XErrorEvent *ee) {
    (void)d;
    if (ee->error_code == BadAccess && ee->resourceid == root) {
        fprintf(stderr, "nyxwm: Gagal berjalan. WM lain sepertinya sedang aktif.\n");
        exit(1);
    }
    return 0;
}

void setup(void) {
    XSetErrorHandler(xerror);
    dpy = XOpenDisplay(NULL);
    if (!dpy) {
        fprintf(stderr, "nyxwm: Gagal membuka X Display.\n");
        exit(1);
    }
    
    root = DefaultRootWindow(dpy);
    int screen = DefaultScreen(dpy);
    sw = DisplayWidth(dpy, screen);
    sh = DisplayHeight(dpy, screen);

    XSelectInput(dpy, root, SubstructureRedirectMask | SubstructureNotifyMask | ButtonPressMask);

    for (int i = 0; i < keys_count; i++) {
        KeyCode code = XKeysymToKeycode(dpy, keys[i].keysym);
        XGrabKey(dpy, code, keys[i].mod, root, True, GrabModeAsync, GrabModeAsync);
    }

    Cursor cursor = XCreateFontCursor(dpy, XC_left_ptr);
    XDefineCursor(dpy, root, cursor);

    wm_check_win = XCreateSimpleWindow(dpy, root, 0, 0, 1, 1, 0, 0, 0);
    Atom net_wm_name = XInternAtom(dpy, "_NET_WM_NAME", False);
    Atom net_check = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False);
    Atom utf8 = XInternAtom(dpy, "UTF8_STRING", False);
    
    XChangeProperty(dpy, wm_check_win, net_wm_name, utf8, 8, PropModeReplace, (unsigned char *)"nyxwm", 5);
    XChangeProperty(dpy, root, net_check, XA_WINDOW, 32, PropModeReplace, (unsigned char *)&wm_check_win, 1);
    XChangeProperty(dpy, wm_check_win, net_check, XA_WINDOW, 32, PropModeReplace, (unsigned char *)&wm_check_win, 1);
    XChangeProperty(dpy, root, net_wm_name, utf8, 8, PropModeReplace, (unsigned char *)"nyxwm", 5);

    int margin_x = 12;
    int margin_y = 8;
    int bar_w = sw - (margin_x * 2);
    int bar_total_h = BAR_HEIGHT;

    bar_win = XCreateSimpleWindow(dpy, root, margin_x, margin_y, bar_w, bar_total_h, 0, 0x1e1e2e, 0x1e1e2e);
    
    XStoreName(dpy, bar_win, "nyxbar");
    
    XSelectInput(dpy, bar_win, ExposureMask);
    XMapWindow(dpy, bar_win);
    
    XSync(dpy, False);
}

void cleanup(void) {
    XUnmapWindow(dpy, bar_win);
    XDestroyWindow(dpy, bar_win);
    XDestroyWindow(dpy, wm_check_win);
    XUngrabKey(dpy, AnyKey, AnyModifier, root);
    XCloseDisplay(dpy);
}

int main(void) {
    setup();
    drawbar();

    int x11_fd = ConnectionNumber(dpy);
    fd_set fds;
    struct timeval tv;

    while (running) {
        while (XPending(dpy)) {
            XEvent ev;
            XNextEvent(dpy, &ev);
            handle_event(&ev);
        }

        if (!running) break;

        FD_ZERO(&fds);
        FD_SET(x11_fd, &fds);
        
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        select(x11_fd + 1, &fds, NULL, NULL, &tv);

        drawbar();
    }

    cleanup();
    return 0;
}
