#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "config.h"

extern Display *dpy;
extern Window root;
extern Window bar_win;
extern int sw, sh;
extern int xi_opcode;
extern XFontStruct *bar_font;

Client *clients = NULL;
Client *sel = NULL;
int current_tag = 0;
int running = 1;
int current_layout = 0;

int canvas_mode[4] = {0, 0, 0, 0};
int cam_x[4] = {0, 0, 0, 0};
int cam_y[4] = {0, 0, 0, 0};
Window map_win = None;

void draw_minimap(void) {
    if (!map_win || !canvas_mode[current_tag]) return;
    XClearWindow(dpy, map_win);
    GC gc = XCreateGC(dpy, map_win, 0, NULL);
    
    int vcx = cam_x[current_tag] + sw / 2;
    int vcy = cam_y[current_tag] + sh / 2;
    double sx = 200.0 / (8.0 * sw);
    double sy = 150.0 / (8.0 * sh);
    
    int v_x = 100 + (int)((cam_x[current_tag] - vcx) * sx);
    int v_y = 75 + (int)((cam_y[current_tag] - vcy) * sy);
    int v_w = (int)(sw * sx);
    int v_h = (int)(sh * sy);
    
    XSetForeground(dpy, gc, 0x585b70);
    XDrawRectangle(dpy, map_win, gc, v_x, v_y, v_w, v_h);
    
    for (Client *c = clients; c; c = c->next) {
        if (c->tag == current_tag) {
            XWindowAttributes wa;
            XGetWindowAttributes(dpy, c->win, &wa);
            int ax = wa.x + cam_x[current_tag];
            int ay = wa.y + cam_y[current_tag];
            
            int cx = 100 + (int)((ax - vcx) * sx);
            int cy = 75 + (int)((ay - vcy) * sy);
            int cw = (int)(wa.width * sx);
            int ch = (int)(wa.height * sy);
            
            if (cw < 2) cw = 2;
            if (ch < 2) ch = 2;
            
            if (c == sel) {
                XSetForeground(dpy, gc, 0x89b4fa);
                XFillRectangle(dpy, map_win, gc, cx, cy, cw, ch);
            } else {
                XSetForeground(dpy, gc, 0xf5c2e7);
                XDrawRectangle(dpy, map_win, gc, cx, cy, cw, ch);
            }
        }
    }
    XFreeGC(dpy, gc);
}

void drawbar(void) {
    if (!SHOW_BAR) return;

    XWindowAttributes wa;
    XGetWindowAttributes(dpy, bar_win, &wa);
    int bar_h = wa.height;
    int bar_w = wa.width;

    XGCValues gcv;
    GC gc = XCreateGC(dpy, bar_win, 0, &gcv);
    
    if (bar_font) {
        XSetFont(dpy, gc, bar_font->fid);
    }
    
    XSetForeground(dpy, gc, 0x11111b);
    XFillRectangle(dpy, bar_win, gc, 0, 0, bar_w, bar_h);
    
    int text_y = bar_font ? (bar_h + bar_font->ascent - bar_font->descent) / 2 : (bar_h / 2) + 4;
    
    for (int i = 0; i < 4; i++) {
        char label[2] = { '1' + i, '\0' };
        int box_w = 24;
        int box_h = bar_h - 8;
        int box_x = 5 + i * 30;
        int box_y = 4;
        
        if (i == current_tag) {
            XSetForeground(dpy, gc, 0x89b4fa);
            XFillRectangle(dpy, bar_win, gc, box_x, box_y, box_w, box_h);
            XSetForeground(dpy, gc, 0x11111b);
        } else {
            XSetForeground(dpy, gc, 0xa6adc8);
        }
        
        int tw = bar_font ? XTextWidth(bar_font, label, 1) : 6;
        int tx = box_x + (box_w - tw) / 2;
        XDrawString(dpy, bar_win, gc, tx, text_y, label, 1);
    }
    
    if (sel) {
        char *name = NULL;
        if (XFetchName(dpy, sel->win, &name) && name) {
            XWindowAttributes win_wa;
            XGetWindowAttributes(dpy, sel->win, &win_wa);
            
            int abs_x = win_wa.x + cam_x[current_tag];
            int abs_y = win_wa.y + cam_y[current_tag];
            
            char title_buf[512];
            snprintf(title_buf, sizeof(title_buf), "%s [%d, %d]", name, abs_x, abs_y);
            
            XSetForeground(dpy, gc, 0xcdd6f4);
            XDrawString(dpy, bar_win, gc, 140, text_y, title_buf, strlen(title_buf));
            XFree(name);
        }
    }
    
    char cpu_str[16], ram_str[32], time_str[64], status_text[256];
    extern void get_cpu(char*, size_t);
    extern void get_ram(char*, size_t);
    
    get_cpu(cpu_str, sizeof(cpu_str));
    get_ram(ram_str, sizeof(ram_str));

    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(time_str, sizeof(time_str), "%a, %d %b • %H:%M", tm_info);

    if (canvas_mode[current_tag]) {
        snprintf(status_text, sizeof(status_text), " [CANVAS] nyxwm-1.0  |    %s  |    %s  |    %s ", cpu_str, ram_str, time_str);
    } else {
        snprintf(status_text, sizeof(status_text), " nyxwm-1.1  |    %s  |    %s  |    %s ", cpu_str, ram_str, time_str);
    }

    XSetForeground(dpy, gc, 0xf5c2e7);
    int tw = bar_font ? XTextWidth(bar_font, status_text, strlen(status_text)) : (int)(strlen(status_text) * 6);
    int tx = bar_w - tw - 10;
    XDrawString(dpy, bar_win, gc, tx, text_y, status_text, strlen(status_text));
    
    XFreeGC(dpy, gc);
    
    if (canvas_mode[current_tag]) {
        draw_minimap();
    }
}

void tile(void) {
    Client *c;
    int n = 0;

    for (c = clients; c; c = c->next) {
        if (c->tag != current_tag) {
            if (c->isfloating) {
                XWindowAttributes wa;
                XGetWindowAttributes(dpy, c->win, &wa);
                if (wa.x >= 0) {
                    long coords[2] = {wa.x + cam_x[current_tag], wa.y + cam_y[current_tag]};
                    Atom prop = XInternAtom(dpy, "_NYX_FLOAT_POS", False);
                    XChangeProperty(dpy, c->win, prop, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)coords, 2);
                }
            }
            XMoveWindow(dpy, c->win, -sw * 2, 0);
        }
    }

    for (c = clients; c; c = c->next) {
        if (c->tag == current_tag && c->isfloating) {
            Atom prop = XInternAtom(dpy, "_NYX_FLOAT_POS", False);
            Atom actual_type;
            int actual_format;
            unsigned long nitems, bytes_after;
            unsigned char *prop_to_get = NULL;
            int fx = sw / 4, fy = sh / 4;
            
            if (XGetWindowProperty(dpy, c->win, prop, 0, 2, False, XA_CARDINAL,
                                   &actual_type, &actual_format, &nitems, &bytes_after, &prop_to_get) == Success && prop_to_get) {
                if (nitems == 2) {
                    fx = ((long *)prop_to_get)[0];
                    fy = ((long *)prop_to_get)[1];
                }
                XFree(prop_to_get);
            }
            XMoveWindow(dpy, c->win, fx - cam_x[current_tag], fy - cam_y[current_tag]);
        }
    }

    for (c = clients; c; c = c->next) {
        if (c->tag == current_tag && !c->isfloating) n++;
    }

    if (n == 0) return;

    int current_bar_h = SHOW_BAR ? BAR_HEIGHT : 0;
    int current_bar_pad = SHOW_BAR ? (BAR_PADDING_Y * 2) : 0;

    int mx = OUTER_GAP;
    int my = current_bar_h + OUTER_GAP + current_bar_pad;
    int mh = sh - current_bar_h - (OUTER_GAP * 2) - current_bar_pad;

    if (current_layout == 0) {
        int mw = (n > 1) ? (sw - (OUTER_GAP * 2) - INNER_GAP) * MASTER_SIZE : sw - (OUTER_GAP * 2);
        int sx = mx + mw + INNER_GAP;
        int sy = my;
        int sw_stack = sw - mw - (OUTER_GAP * 2) - INNER_GAP;
        int sh_stack = (n > 1) ? (mh - (INNER_GAP * (n - 2))) / (n - 1) : 0;

        int i = 0;
        for (c = clients; c; c = c->next) {
            if (c->tag != current_tag || c->isfloating) continue;
            if (i == 0) {
                XMoveResizeWindow(dpy, c->win, mx - cam_x[current_tag], my - cam_y[current_tag], mw, mh);
            } else {
                XMoveResizeWindow(dpy, c->win, sx - cam_x[current_tag], sy + ((i - 1) * (sh_stack + INNER_GAP)) - cam_y[current_tag], sw_stack, sh_stack);
            }
            i++;
        }
    } else if (current_layout == 1) {
        int mw = sw - (OUTER_GAP * 2);
        for (c = clients; c; c = c->next) {
            if (c->tag != current_tag || c->isfloating) continue;
            XMoveResizeWindow(dpy, c->win, mx - cam_x[current_tag], my - cam_y[current_tag], mw, mh);
        }
    } else if (current_layout == 2) {
        int cols = 1;
        while (cols * cols < n) cols++;
        int rows = (n + cols - 1) / cols;
        int ww = (sw - (OUTER_GAP * 2) - ((cols - 1) * INNER_GAP)) / cols;
        int wh = (mh - ((rows - 1) * INNER_GAP)) / rows;
        int i = 0;
        for (c = clients; c; c = c->next) {
            if (c->tag != current_tag || c->isfloating) continue;
            int col = i % cols;
            int row = i / cols;
            int gx = mx + col * (ww + INNER_GAP);
            int gy = my + row * (wh + INNER_GAP);
            XMoveResizeWindow(dpy, c->win, gx - cam_x[current_tag], gy - cam_y[current_tag], ww, wh);
            i++;
        }
    }
}

void fn_spawn(const char **arg) {
    extern void spawn(const char **arg);
    spawn(arg);
}

void fn_kill(const char **arg) {
    (void)arg;
    if (!sel) return;
    XKillClient(dpy, sel->win);
}

void fn_focus(const char **arg) {
    (void)arg;
    if (!clients) return;
    if (sel && sel->next) {
        sel = sel->next;
    } else {
        sel = clients;
    }
    while (sel && sel->tag != current_tag) {
        sel = sel->next;
        if (!sel) sel = clients;
    }
    if (sel) {
        XSetInputFocus(dpy, sel->win, RevertToParent, CurrentTime);
        XRaiseWindow(dpy, sel->win);
    }
    drawbar();
}

void fn_toggle_float(const char **arg) {
    (void)arg;
    if (!sel) return;
    sel->isfloating = !sel->isfloating;
    if (sel->isfloating) {
        int fx = sw / 4 + cam_x[current_tag];
        int fy = sh / 4 + cam_y[current_tag];
        XMoveResizeWindow(dpy, sel->win, fx - cam_x[current_tag], fy - cam_y[current_tag], sw / 2, sh / 2);
        
        long coords[2] = {fx, fy};
        Atom prop = XInternAtom(dpy, "_NYX_FLOAT_POS", False);
        XChangeProperty(dpy, sel->win, prop, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)coords, 2);
    } else {
        tile();
    }
}

void fn_set_tag(const char **arg) {
    current_tag = atoi(arg[0]);
    tile();
    sel = NULL;
    for (Client *c = clients; c; c = c->next) {
        if (c->tag == current_tag) {
            sel = c;
            XSetInputFocus(dpy, sel->win, RevertToParent, CurrentTime);
            XRaiseWindow(dpy, sel->win);
            break;
        }
    }
    drawbar();
}

void fn_alt_tab(const char **arg) {
    fn_focus(arg);
}

void fn_next_layout(const char **arg) {
    (void)arg;
    current_layout = (current_layout + 1) % 3;
    tile();
    drawbar();
}

void fn_toggle_canvas(const char **arg) {
    (void)arg;
    canvas_mode[current_tag] = !canvas_mode[current_tag];
    
    if (canvas_mode[current_tag]) {
        if (!map_win) {
            int mw = 200;
            int mh = 150;
            int mx = sw - mw - OUTER_GAP;
            int my = BAR_HEIGHT + (BAR_PADDING_Y * 2) + OUTER_GAP;
            map_win = XCreateSimpleWindow(dpy, root, mx, my, mw, mh, 1, 0x89b4fa, 0x11111b);
            Atom opacity_atom = XInternAtom(dpy, "_NET_WM_WINDOW_OPACITY", False);
            unsigned long opacity = (unsigned long)(0.75 * 4294967295.0);
            XChangeProperty(dpy, map_win, opacity_atom, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)&opacity, 1);
            XMapWindow(dpy, map_win);
        }
        for (Client *c = clients; c; c = c->next) {
            if (c->tag == current_tag) {
                c->isfloating = 1;
                
                XWindowAttributes wa;
                XGetWindowAttributes(dpy, c->win, &wa);
                
                long coords[2] = {wa.x + cam_x[current_tag], wa.y + cam_y[current_tag]};
                Atom prop = XInternAtom(dpy, "_NYX_FLOAT_POS", False);
                XChangeProperty(dpy, c->win, prop, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)coords, 2);
            }
        }
    } else {
        if (map_win) {
            XUnmapWindow(dpy, map_win);
            XDestroyWindow(dpy, map_win);
            map_win = None;
        }
        cam_x[current_tag] = 0;
        cam_y[current_tag] = 0;
        for (Client *c = clients; c; c = c->next) {
            if (c->tag == current_tag) {
                c->isfloating = 0;
            }
        }
    }
    tile();
    drawbar();
}

void fn_screenshot(const char **arg) {
    (void)arg;
    if (XGrabPointer(dpy, root, False, ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime) != GrabSuccess) return;
    
    XEvent ev;
    int start_x = 0, start_y = 0, end_x = 0, end_y = 0;
    int drawing = 0;
    
    GC gc = XCreateGC(dpy, root, 0, NULL);
    XSetFunction(dpy, gc, GXxor);
    XSetForeground(dpy, gc, 0xffffff);
    XSetSubwindowMode(dpy, gc, IncludeInferiors);
    
    int running_ss = 1;
    while (running_ss) {
        XNextEvent(dpy, &ev);
        if (ev.type == ButtonPress && ev.xbutton.button == Button1) {
            start_x = ev.xbutton.x_root;
            start_y = ev.xbutton.y_root;
            end_x = start_x;
            end_y = start_y;
            drawing = 1;
        } else if (ev.type == MotionNotify && drawing) {
            XDrawRectangle(dpy, root, gc, start_x < end_x ? start_x : end_x, start_y < end_y ? start_y : end_y, abs(end_x - start_x), abs(end_y - start_y));
            end_x = ev.xmotion.x_root;
            end_y = ev.xmotion.y_root;
            XDrawRectangle(dpy, root, gc, start_x < end_x ? start_x : end_x, start_y < end_y ? start_y : end_y, abs(end_x - start_x), abs(end_y - start_y));
        } else if (ev.type == ButtonRelease && ev.xbutton.button == Button1) {
            if (drawing) {
                XDrawRectangle(dpy, root, gc, start_x < end_x ? start_x : end_x, start_y < end_y ? start_y : end_y, abs(end_x - start_x), abs(end_y - start_y));
            }
            running_ss = 0;
        }
    }
    
    XFreeGC(dpy, gc);
    XUngrabPointer(dpy, CurrentTime);
    
    int rx = start_x < end_x ? start_x : end_x;
    int ry = start_y < end_y ? start_y : end_y;
    int rw = abs(end_x - start_x);
    int rh = abs(end_y - start_y);
    
    if (rw > 5 && rh > 5) {
        XImage *img = XGetImage(dpy, root, rx, ry, rw, rh, AllPlanes, ZPixmap);
        if (img) {
            char *home = getenv("HOME");
            if (home) {
                char path[512];
                snprintf(path, sizeof(path), "%s/Pictures", home);
                mkdir(path, 0755);
                snprintf(path, sizeof(path), "%s/Pictures/Screenshots", home);
                mkdir(path, 0755);
                
                time_t t = time(NULL);
                struct tm *tm = localtime(&t);
                char filepath[1024];
                snprintf(filepath, sizeof(filepath), "%s/%04d-%02d-%02d_%02d-%02d-%02d.png", path,
                         tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                         tm->tm_hour, tm->tm_min, tm->tm_sec);
                
                unsigned char *rgb_data = malloc(rw * rh * 3);
                if (rgb_data) {
                    for (int y = 0; y < rh; y++) {
                        for (int x = 0; x < rw; x++) {
                            unsigned long pixel = XGetPixel(img, x, y);
                            rgb_data[(y * rw + x) * 3 + 0] = (pixel & 0xff0000) >> 16;
                            rgb_data[(y * rw + x) * 3 + 1] = (pixel & 0x00ff00) >> 8;
                            rgb_data[(y * rw + x) * 3 + 2] = (pixel & 0x0000ff);
                        }
                    }
                    stbi_write_png(filepath, rw, rh, 3, rgb_data, rw * 3);
                    free(rgb_data);
                }
            }
            XDestroyImage(img);
        }
    }
}

void fn_quit(const char **arg) {
    (void)arg;
    running = 0;
}

void manage(Window w) {
    if (w == bar_win) return;
    Client *c = malloc(sizeof(Client));
    c->win = w;
    c->isfloating = canvas_mode[current_tag];
    c->tag = current_tag;
    c->next = clients;
    clients = c;
    sel = c;
    
    XSelectInput(dpy, w, FocusChangeMask | PropertyChangeMask);
    
    XGrabButton(dpy, Button1, AnyModifier, w, False, ButtonPressMask, 
                GrabModeSync, GrabModeAsync, None, None);
    XGrabButton(dpy, Button2, AnyModifier, w, False, ButtonPressMask, 
                GrabModeSync, GrabModeAsync, None, None);
    XGrabButton(dpy, Button3, AnyModifier, w, False, ButtonPressMask, 
                GrabModeSync, GrabModeAsync, None, None);

    XMapWindow(dpy, w);
    XSetInputFocus(dpy, w, RevertToParent, CurrentTime);
    tile();
    drawbar();
}

void unmanage(Window w) {
    if (w == bar_win) return;
    Client *c, *prev = NULL;
    for (c = clients; c; prev = c, c = c->next) {
        if (c->win == w) {
            if (prev) prev->next = c->next;
            else clients = c->next;
            if (sel == c) {
                sel = clients;
                while (sel && sel->tag != current_tag) {
                    sel = sel->next;
                }
            }
            free(c);
            break;
        }
    }
    if (sel) {
        XSetInputFocus(dpy, sel->win, RevertToParent, CurrentTime);
    } else {
        XSetInputFocus(dpy, root, RevertToParent, CurrentTime);
    }
    tile();
    drawbar();
}

void handle_event(XEvent *ev) {
    switch (ev->type) {
        case GenericEvent: {
            if (ev->xcookie.extension == xi_opcode) {
                if (XGetEventData(dpy, &ev->xcookie)) {
                    int evtype = ev->xcookie.evtype;
                    
                    if (evtype == XI_GestureSwipeBegin || 
                        evtype == XI_GestureSwipeUpdate || 
                        evtype == XI_GestureSwipeEnd) {
                        
                        XIGestureSwipeEvent *gev = (XIGestureSwipeEvent *)ev->xcookie.data;
                        
                        if (gev->detail == 3 && canvas_mode[current_tag]) {
                            if (evtype == XI_GestureSwipeUpdate) {
                                cam_x[current_tag] -= (int)gev->delta_x;
                                cam_y[current_tag] -= (int)gev->delta_y;
                                
                                tile();
                                drawbar();
                            }
                        }
                        
                        if (gev->detail == 4) {
                            Client *c = NULL;
                            if (gev->child != None) {
                                for (Client *t = clients; t; t = t->next) {
                                    if (t->win == gev->child) {
                                        c = t;
                                        break;
                                    }
                                }
                            }
                            if (!c) c = sel;
                            
                            if (c && c->isfloating) {
                                if (evtype == XI_GestureSwipeBegin) {
                                    sel = c;
                                    XSetInputFocus(dpy, sel->win, RevertToParent, CurrentTime);
                                    XRaiseWindow(dpy, sel->win);
                                    drawbar();
                                } else if (evtype == XI_GestureSwipeUpdate) {
                                    XWindowAttributes wa;
                                    XGetWindowAttributes(dpy, c->win, &wa);
                                    int nx = wa.x + (int)gev->delta_x;
                                    int ny = wa.y + (int)gev->delta_y;
                                    
                                    XMoveWindow(dpy, c->win, nx, ny);
                                    
                                    long coords[2] = {nx + cam_x[current_tag], ny + cam_y[current_tag]};
                                    Atom prop = XInternAtom(dpy, "_NYX_FLOAT_POS", False);
                                    XChangeProperty(dpy, c->win, prop, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)coords, 2);
                                    
                                    drawbar();
                                }
                            }
                        }
                    }
                    XFreeEventData(dpy, &ev->xcookie);
                }
            }
            break;
        }
        case Expose:
            if (ev->xexpose.window == bar_win && ev->xexpose.count == 0) {
                drawbar();
            }
            break;
        case PropertyNotify:
            if (ev->xproperty.window == root && ev->xproperty.atom == XA_WM_NAME) {
                drawbar();
            }
            break;
        case MapRequest:
            manage(ev->xmaprequest.window);
            break;
        case DestroyNotify:
            unmanage(ev->xdestroywindow.window);
            break;
        case UnmapNotify:
            unmanage(ev->xunmap.window);
            break;
        case ConfigureRequest: {
            XConfigureRequestEvent *cre = &ev->xconfigurerequest;
            XWindowChanges wc;
            wc.x = cre->x;
            wc.y = cre->y;
            wc.width = cre->width;
            wc.height = cre->height;
            wc.border_width = cre->border_width;
            wc.sibling = cre->above;
            wc.stack_mode = cre->detail;
            XConfigureWindow(dpy, cre->window, cre->value_mask, &wc);

            Client *c;
            for (c = clients; c; c = c->next) {
                if (c->win == cre->window) {
                    if (!c->isfloating && c->tag == current_tag) {
                        tile();
                    }
                    break;
                }
            }
            break;
        }
        case ButtonPress: {
            XButtonEvent *be = &ev->xbutton;
            Window win = be->window;
            Client *c = NULL;
            
            for (Client *t = clients; t; t = t->next) {
                if (t->win == win) {
                    c = t;
                    break;
                }
            }

            if (c && sel != c) {
                sel = c;
                XSetInputFocus(dpy, sel->win, RevertToParent, CurrentTime);
                XRaiseWindow(dpy, sel->win);
                drawbar();
            }

            static Time last_button2_time = 0;
            if (be->button == Button2) {
                if (be->time - last_button2_time < 300) {
                    const char *kitty_cmd[] = {"kitty", NULL};
                    fn_spawn(kitty_cmd);
                    XAllowEvents(dpy, AsyncPointer, be->time);
                    last_button2_time = 0;
                    break;
                } else {
                    last_button2_time = be->time;
                    XAllowEvents(dpy, ReplayPointer, be->time);
                    break;
                }
            }

            static Time last_button3_time = 0;
            if (be->button == Button3 && !(be->state & MODKEY)) {
                if (be->time - last_button3_time < 300) {
                    fn_kill(NULL);
                    XAllowEvents(dpy, AsyncPointer, be->time);
                    last_button3_time = 0;
                    break;
                } else {
                    last_button3_time = be->time;
                    XAllowEvents(dpy, ReplayPointer, be->time);
                    break;
                }
            }
            
            if ((be->state & MODKEY) && (be->state & ShiftMask) && be->button == Button1) {
                if (canvas_mode[current_tag]) {
                    XEvent mouse_ev;
                    int start_x = be->x_root;
                    int start_y = be->y_root;
                    int start_cam_x = cam_x[current_tag];
                    int start_cam_y = cam_y[current_tag];
                    
                    if (XGrabPointer(dpy, root, False, PointerMotionMask | ButtonReleaseMask,
                                     GrabModeAsync, GrabModeAsync, None, None, CurrentTime) == GrabSuccess) {
                        while (1) {
                            XNextEvent(dpy, &mouse_ev);
                            if (mouse_ev.type == MotionNotify) {
                                int dx = mouse_ev.xmotion.x_root - start_x;
                                int dy = mouse_ev.xmotion.y_root - start_y;
                                cam_x[current_tag] = start_cam_x - dx;
                                cam_y[current_tag] = start_cam_y - dy;
                                tile();
                                drawbar();
                            } else if (mouse_ev.type == ButtonRelease && mouse_ev.xbutton.button == Button1) {
                                break;
                            }
                        }
                        XUngrabPointer(dpy, CurrentTime);
                    }
                }
            }
            else if ((be->state & MODKEY) && !(be->state & ShiftMask) && be->button == Button1 && c && c->isfloating) {
                XEvent mouse_ev;
                int start_x = be->x_root;
                int start_y = be->y_root;
                XWindowAttributes wa;
                XGetWindowAttributes(dpy, c->win, &wa);
                int start_win_x = wa.x;
                int start_win_y = wa.y;
                
                if (XGrabPointer(dpy, root, False, PointerMotionMask | ButtonReleaseMask,
                                 GrabModeAsync, GrabModeAsync, None, None, CurrentTime) == GrabSuccess) {
                    while (1) {
                        XNextEvent(dpy, &mouse_ev);
                        if (mouse_ev.type == MotionNotify) {
                            int dx = mouse_ev.xmotion.x_root - start_x;
                            int dy = mouse_ev.xmotion.y_root - start_y;
                            int nx = start_win_x + dx;
                            int ny = start_win_y + dy;
                            
                            XMoveWindow(dpy, c->win, nx, ny);
                            
                            long coords[2] = {nx + cam_x[current_tag], ny + cam_y[current_tag]};
                            Atom prop = XInternAtom(dpy, "_NYX_FLOAT_POS", False);
                            XChangeProperty(dpy, c->win, prop, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)coords, 2);
                            drawbar();
                        } else if (mouse_ev.type == ButtonRelease && mouse_ev.xbutton.button == Button1) {
                            break;
                        }
                    }
                    XUngrabPointer(dpy, CurrentTime);
                }
            }
            else if ((be->state & MODKEY) && be->button == Button3 && c && c->isfloating) {
                XEvent mouse_ev;
                int start_x = be->x_root;
                int start_y = be->y_root;
                XWindowAttributes wa;
                XGetWindowAttributes(dpy, c->win, &wa);
                int start_win_w = wa.width;
                int start_win_h = wa.height;
                
                if (XGrabPointer(dpy, root, False, PointerMotionMask | ButtonReleaseMask,
                                 GrabModeAsync, GrabModeAsync, None, None, CurrentTime) == GrabSuccess) {
                    while (1) {
                        XNextEvent(dpy, &mouse_ev);
                        if (mouse_ev.type == MotionNotify) {
                            int dx = mouse_ev.xmotion.x_root - start_x;
                            int dy = mouse_ev.xmotion.y_root - start_y;
                            int nw = start_win_w + dx;
                            int nh = start_win_h + dy;
                            
                            if (nw < 60) nw = 60;
                            if (nh < 60) nh = 60;
                            
                            XResizeWindow(dpy, c->win, nw, nh);
                        } else if (mouse_ev.type == ButtonRelease && mouse_ev.xbutton.button == Button3) {
                            break;
                        }
                    }
                    XUngrabPointer(dpy, CurrentTime);
                }
            }
            XAllowEvents(dpy, ReplayPointer, be->time);
            break;
        }
        case KeyPress: {
            KeySym keysym = XLookupKeysym(&ev->xkey, 0);
            for (int i = 0; i < keys_count; i++) {
                if (keysym == keys[i].keysym && (ev->xkey.state & (keys[i].mod)) == keys[i].mod) {
                    if (keys[i].func) keys[i].func(keys[i].arg);
                }
            }
            break;
        }
    }
}
