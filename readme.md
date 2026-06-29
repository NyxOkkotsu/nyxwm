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

## ⚡ Features

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

* Single `config.h`
* Compile-time configuration
* No runtime parsing
* No garbage collector
* No unnecessary dependencies

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

## Dependencies

```bash
sudo pacman -S libx11 pamixer brightnessctl base-devel
```


## Build & Install

```bash
git clone https://github.com/NyxOkkotsu/nyxwm.git
cd nyxwm
make build 
makepkg -si
```


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


## Configuration — `config.h`

```c
#include <X11/XF86keysym.h>

static const Key keys[] = {

	/* Audio */
	{ 0, XF86XK_AudioRaiseVolume, fn_spawn, (const char*[]){"/bin/sh","-c","pamixer -i 5",NULL} },
	{ 0, XF86XK_AudioLowerVolume, fn_spawn, (const char*[]){"/bin/sh","-c","pamixer -d 5",NULL} },
	{ 0, XF86XK_AudioMute,        fn_spawn, (const char*[]){"/bin/sh","-c","pamixer -t",NULL} },

	/* Brightness */
	{ 0, XF86XK_MonBrightnessUp,   fn_spawn, (const char*[]){"/bin/sh","-c","brightnessctl set +10%",NULL} },
	{ 0, XF86XK_MonBrightnessDown, fn_spawn, (const char*[]){"/bin/sh","-c","brightnessctl set 10%-",NULL} },

};
```

Recompile after changes:

```bash
make build && makepkg -si
```

## Launch

`~/.xinitrc`:

```bash
#!/bin/sh

eval $(dbus-launch --sh-syntax)

exec nyxwm
```

Start:

```bash
startx
```

## License

MIT License.

Do whatever you want with it.
Break it if you can.

