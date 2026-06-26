#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "config.h"

extern Display *dpy;
extern Window root;
extern Window bar_win;
extern int sw, sh;

Client *clients = NULL;
Client *sel = NULL;
int current_tag = 0;
int running = 1;

int canvas_mode[4] = {0, 0, 0, 0};
int cam_x[4] = {0, 0, 0, 0};
int cam_y[4] = {0, 0, 0, 0};

void drawbar(void) {
    XWindowAttributes wa;
    XGetWindowAttributes(dpy, bar_win, &wa);
    int bar_h = wa.height;
    int bar_w = wa.width;

    XGCValues gcv;
    GC gc = XCreateGC(dpy, bar_win, 0, &gcv);
    
    XFontStruct *font = XLoadQueryFont(dpy, "fixed");
    if (font) {
        XSetFont(dpy, gc, font->fid);
    }
    
    XSetForeground(dpy, gc, 0x11111b);
    XFillRectangle(dpy, bar_win, gc, 0, 0, bar_w, bar_h);
    
    int text_y = font ? (bar_h + font->ascent - font->descent) / 2 : (bar_h / 2) + 4;
    
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
        
        int tw = font ? XTextWidth(font, label, 1) : 6;
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
    
    static unsigned long long p_user, p_nice, p_system, p_idle, p_iowait, p_irq, p_softirq;
    unsigned long long user, nice, system, idle, iowait, irq, softirq;
    int cpu_usage = 0;
    
    FILE *f_cpu = fopen("/proc/stat", "r");
    if (f_cpu) {
        char buf[256];
        if (fgets(buf, sizeof(buf), f_cpu)) {
            sscanf(buf, "cpu %llu %llu %llu %llu %llu %llu %llu", &user, &nice, &system, &idle, &iowait, &irq, &softirq);
            unsigned long long p_act = p_user + p_nice + p_system + p_irq + p_softirq;
            unsigned long long p_tot = p_act + p_idle + p_iowait;
            unsigned long long act = user + nice + system + irq + softirq;
            unsigned long long tot = act + idle + iowait;
            if ((tot - p_tot) > 0) {
                cpu_usage = (int)(((act - p_act) * 100) / (tot - p_tot));
            }
            p_user = user; p_nice = nice; p_system = system; p_idle = idle;
            p_iowait = iowait; p_irq = irq; p_softirq = softirq;
        }
        fclose(f_cpu);
    }

    unsigned long long total_mem = 0, avail_mem = 0;
    FILE *f_mem = fopen("/proc/meminfo", "r");
    if (f_mem) {
        char line[256];
        while (fgets(line, sizeof(line), f_mem)) {
            if (strncmp(line, "MemTotal:", 9) == 0) sscanf(line, "MemTotal: %llu", &total_mem);
            else if (strncmp(line, "MemAvailable:", 13) == 0) sscanf(line, "MemAvailable: %llu", &avail_mem);
        }
        fclose(f_mem);
    }
    double used_gb = (double)(total_mem - avail_mem) / 1024.0 / 1024.0;
    double total_gb = (double)total_mem / 1024.0 / 1024.0;
    char ram_str[32];
    snprintf(ram_str, sizeof(ram_str), "%.1fG/%.1fG", used_gb, total_gb);

    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%a, %d %b • %H:%M", tm_info);

    char status_text[256];
    if (canvas_mode[current_tag]) {
        snprintf(status_text, sizeof(status_text), " [CANVAS] nyxwm-1.0  |    %d%%  |    %s  |    %s ", cpu_usage, ram_str, time_str);
    } else {
        snprintf(status_text, sizeof(status_text), " nyxwm-1.0  |    %d%%  |    %s  |    %s ", cpu_usage, ram_str, time_str);
    }

    XSetForeground(dpy, gc, 0xf5c2e7);
    int tw = font ? XTextWidth(font, status_text, strlen(status_text)) : (int)(strlen(status_text) * 6);
    int tx = bar_w - tw - 10;
    XDrawString(dpy, bar_win, gc, tx, text_y, status_text, strlen(status_text));
    
    if (font) {
        XFreeFont(dpy, font);
    }
    XFreeGC(dpy, gc);
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

    int mw = (n > 1) ? (sw - (OUTER_GAP * 2) - INNER_GAP) * MASTER_SIZE : sw - (OUTER_GAP * 2);
    int mh = sh - BAR_HEIGHT - (OUTER_GAP * 2) - (BAR_PADDING_Y * 2);
    int mx = OUTER_GAP;
    int my = BAR_HEIGHT + OUTER_GAP + (BAR_PADDING_Y * 2);

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

void fn_toggle_canvas(const char **arg) {
    (void)arg;
    canvas_mode[current_tag] = !canvas_mode[current_tag];
    
    if (canvas_mode[current_tag]) {
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
