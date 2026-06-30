```text
███╗   ██╗██╗   ██╗██╗  ██╗██╗    ██╗███╗   ███╗
████╗  ██║╚██╗ ██╔╝╚██╗██╔╝██║    ██║████╗ ████║
██╔██╗ ██║ ╚████╔╝  ╚███╔╝ ██║ █╗ ██║██╔████╔██║
██║╚██╗██║  ╚██╔╝   ██╔██╗ ██║███╗██║██║╚██╔╝██║
██║ ╚████║   ██║   ██╔╝ ██╗╚███╔███╔╝██║ ╚═╝ ██║
╚═╝  ╚═══╝   ╚═╝   ╚═╝  ╚═╝ ╚══╝╚══╝ ╚═╝     ╚═╝

        nyxwm — no bloat, no mercy.
```

---

# nyxwm

A minimal, high-performance tiling window manager for X11 written in pure C.

Nyx grew tired of tedious window managers that merely showcased different layouts, as well as "unlimited canvas" WMs that—while cool—consumed excessive resources. Consequently, Nyx set out to create a window manager that is lightweight, fast, and efficient without sacrificing functionality; it offers three layouts and supports two desktop modes—tiling and canvas—simultaneously.

Nyx recommends using Picom for an enhanced experience.

Please feel free to provide suggestions or report any issues you encounter.

## Features

Take a look into the features of Nyxwm.

### Blazingly Fast 

* Nyxwm is written in pure C, therefore there is zero abstraction
* Near 0ms startup, provided you have a sufficiently supporting hardware.
* Minimal RAM usage. 
* Stable under heavy workloads (gaming, compiling)

### Tidy Tiling & Grid Symmetries

* Clean, deterministic layouts
* No overlapping chaos
* Every pixel is used efficiently

### Suckless Philosophy

* You are going to change just : `config.h`
* Compile-time configuration
* There are no runtime parsing
* No garbage collector
* Or no unnecessary dependencies except `gcc` and other deps mentioned.

### Built-in Status Bar
Core Elements:

* **Workspaces (Left):**
  Displays active tags/workspaces with clear visual indication of the current focus.

* **Active Window Title (Center/Adjacent):**
  Shows the currently focused window name for quick context switching.

* **System Identifier (Right):**
  Displays a static signature:
  `nyxwm-1.1`

### Unlimited canvas panning desktop
* **Panning Desktop:**
  Activate the unlimited canvas panning desktop with pressing WIN+G and panning with MOD+SHIFT+LEFRDRAGMOUSE

### Built-In Screenshots Tools
* Just press win+shift+s to screenshots area and the image will be saved on ~/Pictures/Screenshots

### 3 Tilling Layout
* You Can Change Tilling Layout By Pressing Win+L, This Wm Has A 3 tilling Layout That You Can Try

### ALT+TAB To Change Window Focus
* Change Window Focus With ALT+TAB

### Window Location Indicator At Canvas Mode (MAPS)
* Automatically Spawn A Minimaps Every Change Into Canvas Mode

### Advanced Shell Engine (`fn_spawn`)

* Non-blocking execution
* Direct `/bin/sh` piping
* Fast command dispatch
* Script + system control ready

### Native Hardware Controls

Pre-mapped keys for:

* Volume → `pamixer`
* Brightness → `brightnessctl`
* Uses XF86 multimedia keys

### PipeWire / WirePlumber Ready

* Modern Linux audio stack compatible
* No DE required
* Clean integration

## Installation :

### Dependencies

Before installing `nyxwm`, you gotta install the dependencies. If you are in an arch based system, use the following command. 


```bash
sudo pacman -S libx11 pamixer brightnessctl base-devel
```


### Build & Install

After you have successfully finished installing the dependencies, you need to clone the repo, build the binary, and install it via `makepkg -si`, which would make the **binary manageable with pacman.**


```bash
git clone https://github.com/NyxOkkotsu/nyxwm.git
cd nyxwm
make build 
makepkg -si
```

### Testing 

If you are willing to contribute to this project, here are a list of `make` commands that would make your workflow feel faster.

**Make commands**

I have added several useful make build instructions.

1) Build the project.

```bash
make build 
# or 
make all
```

2) Run the main binary. 

This one builds the binary and then runs it. Here the build directory is not deleted and persists. 

```bash 
make run
```

3) Test the program 

This one will build the binary and then run it, then finally clean it. No build directory persists. 

```bash 
make test 
```

4) Clean the project structure 

This will remove the build directory and clean the object files. 

```bash 
make clean
```


**Arch `makepkg`**

For installing the binary system wide, if you are in an arch based repo, you need to use the `makepkg` to build and install the binary to standard paths. Run the command as given below the install it. 

```bash 
makepkg -si 
```

>[!IMPORTANT]
> This project CTemplate is only for linux based systems and the PKGBUILD is optional (have it if you are an arch based user), else delete it. This makefile structure has no compactness with windows or mac based or any other non linux OS. 


### Uninstalling nyxwm

Case you need to uninstall the window manager, as stated earlier, it can be easily done via `pacman` **if you install it via `makepkg`**. 

```bash
sudo pacman -Rns nyxwm
```


## Configuring your system 

In order to configure the system, you need to edit the `config.h` that is located in `include/` directory. Here is the default `config.h`.

The configuration comes with three parts. 

### WM Properties

By editing these configuration, you will be able to configure whether or not to show the bar, define the bar height, padding, configure the window manager's gaps, background and foreground of the bar, font name (currently it is fixed), update mod key and more. 

```c
#define SHOW_BAR 1

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
```

### WM Variables and Commands

Each keybinding is mapped to execution of a command. You can speciy the commands as a set of string arrays (type : `*command[]`, or `**command`). 

```c
static const char* termcmd[]  = { "kitty", NULL };
static const char* menucmd[]  = { "rofi", "-show", "drun", NULL };
static const char* wallcmd[]  = { "/bin/sh", "-c", "~/rofi-wallpaper.sh", NULL };
static const char *upvol[]      = { "/bin/sh", "-c", "~/volume.sh i", NULL };
static const char *downvol[]    = { "/bin/sh", "-c", "~/volume.sh d", NULL };
static const char *mutevol[]    = { "/bin/sh", "-c", "~/volume.sh t", NULL };
static const char *upbright[]   = { "/bin/sh", "-c", "~/brightness.sh +10%", NULL };
static const char *downbright[] = { "/bin/sh", "-c", "~/brightness.sh 10%-", NULL };
```

### Configure the Tags (Workspaces)

Nyxwm comes preconfigured with 4 workspaces(tags). If you want more than 4, feel free to add it. 

```c 
static const char* tag1[] = { "0", NULL };
static const char* tag2[] = { "1", NULL };
static const char* tag3[] = { "2", NULL };
static const char* tag4[] = { "3", NULL };

```

### Configure the keybindings 

Remember? Previously we had the commands stored in arrays? We use those variables and map them to a keybinding. You can add as many as commands you can and as many as keybindings you can. 

Keybindings are configured as an array of Key type. Where each key is a `struct` : 

```c 
typedef struct {
    unsigned int mod;
    KeySym keysym;
    void (*func)(const char **arg);
    const char **arg;
} Key;

```

**You can add or edit your keybindings here**

```c
static Key keys[] = {
    { MODKEY, XK_Return, fn_spawn, termcmd },
    { MODKEY, XK_p,      fn_spawn, menucmd },
    { MODKEY, XK_w,      fn_spawn, wallcmd },
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
    { MODKEY,               XK_g,      fn_toggle_canvas, NULL },
    { MODKEY | ShiftMask,   XK_s,      fn_screenshot,    NULL },
    { Mod1Mask,             XK_Tab,    fn_alt_tab,       NULL },
    { MODKEY,               XK_l,      fn_next_layout,   NULL },
};

```


> [!IMPORTANT]
> You should not forget to recompile the WM after changes. If else, the binary will stay intact and no changes will be visible. 

```bash
make build && makepkg -si
```

### Launch

To launch the nyxwm, you need to edit the `~/.xinitrc`. But before that, you need to have `xorg-xinit` package installed in your system. Without that, you would not be able to get `startx` command. 

**Install the package**

```bash
sudo pacman -S xorg xorg-xinit 
```


**Edit the `~/.xinitrc` file to start nyxwm :**

`~/.xinitrc`:

```bash
#!/bin/sh

eval $(dbus-launch --sh-syntax)

exec nyxwm
```

**Start nyxwm by running `startx` :**:

```bash
startx
```

## License

This is a licensed project. You are required to follow the LICENSE.
MIT License. Do whatever you want with it. Break it if you can.

