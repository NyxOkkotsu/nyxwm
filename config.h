#ifndef CONFIG_H
#define CONFIG_H

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>

#define BAR_HEIGHT 24
#define BAR_PADDING_X 12
#define BAR_PADDING_Y 6
#define INNER_GAP 10
#define OUTER_GAP 14

#define BG_COLOR "#1e1e2e"
#define FG_COLOR "#cdd6f4"

#define FONT_NAME "fixed"

#define REFRESH_INTERVAL 1
#define MASTER_SIZE 0.60
#define MODKEY Mod4Mask
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

typedef struct Client Client;
struct Client {
    Window win;
    int isfloating;
    int tag;
    Client *next;
};

typedef struct {
    unsigned int mod;
    KeySym keysym;
    void (*func)(const char **arg);
    const char **arg;
} Key;

static const char* termcmd[]  = { "kitty", NULL };
static const char* menucmd[]  = { "rofi", "-show", "drun", NULL };
static const char* tag1[] __attribute__((unused)) = { "0", NULL };
static const char* tag2[] __attribute__((unused)) = { "1", NULL };
static const char* tag3[] __attribute__((unused)) = { "2", NULL };
static const char* tag4[] __attribute__((unused)) = { "3", NULL };

static const char* tags[] __attribute__((unused)) = { "1", "2", "3", "4" };

static const char *upvol[]      = { "/usr/bin/pamixer", "-i", "5", NULL };
static const char *downvol[]    = { "/usr/bin/pamixer", "-d", "5", NULL };
static const char *mutevol[]    = { "/usr/bin/pamixer", "-t", NULL };
static const char *upbright[]   = { "/usr/bin/brightnessctl", "set", "+10%", NULL };
static const char *downbright[] = { "/usr/bin/brightnessctl", "set", "10%-", NULL };

void fn_spawn(const char **arg);
void fn_kill(const char **arg);
void fn_focus(const char **arg);
void fn_toggle_float(const char **arg);
void fn_set_tag(const char **arg);
void fn_quit(const char **arg);

static Key keys[] __attribute__((unused)) = {
    { MODKEY, XK_Return, fn_spawn, termcmd },
    { MODKEY, XK_p,      fn_spawn, menucmd },
    { MODKEY, XK_c,      fn_kill,  NULL },
    { MODKEY, XK_j,      fn_focus, NULL },
    { MODKEY, XK_f,      fn_toggle_float, NULL },
    { MODKEY, XK_q,      fn_quit,  NULL },
    { MODKEY, XK_1,      fn_set_tag, tag1 },
    { MODKEY, XK_2,      fn_set_tag, tag2 },
    { MODKEY, XK_3,      fn_set_tag, tag3 },
    { MODKEY, XK_4,      fn_set_tag, tag4 },
    { 0, XF86XK_AudioRaiseVolume, fn_spawn, upvol },
    { 0, XF86XK_AudioLowerVolume, fn_spawn, downvol },
    { 0, XF86XK_AudioMute, fn_spawn, mutevol },
    { 0, XF86XK_MonBrightnessUp, fn_spawn, upbright },
    { 0, XF86XK_MonBrightnessDown, fn_spawn, downbright },
};
static int keys_count __attribute__((unused)) = sizeof(keys) / sizeof(keys[0]);

#endif
