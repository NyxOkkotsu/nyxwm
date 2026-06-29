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

---

## ⚡ Features

### 🧠 Blazingly Fast 

* Pure C, zero abstraction
* Near 0ms startup
* Microscopic RAM usage
* Stable under heavy workloads (gaming, compiling)

---

### 🧩 Tidy Tiling & Grid Symmetries

* Clean, deterministic layouts
* No overlapping chaos
* Every pixel is used efficiently

---

### ⚙️ Suckless Philosophy

* Single `config.h`
* Compile-time configuration
* No runtime parsing
* No garbage collector
* No unnecessary dependencies

---

### 📊 Built-in Status Bar
Core Elements:

* **Workspaces (Left):**
  Displays active tags/workspaces with clear visual indication of the current focus.

* **Active Window Title (Center/Adjacent):**
  Shows the currently focused window name for quick context switching.

* **System Identifier (Right):**
  Displays a static signature:
  `nyxwm-1.1`

### 📺 Unlimited canvas panning desktop
* **Panning Desktop:**
  Activate the unlimited canvas panning desktop with pressing WIN+G and panning with MOD+SHIFT+LEFRDRAGMOUSE

### 📸 Built-In Screenshots Tools
* Just press win+shift+s to screenshots area and the image will be saved on ~/Pictures/Screenshots

### 🖵 3 Tilling Layout
* You Can Change Tilling Layout By Pressing Win+L, This Wm Has A 3 tilling Layout That You Can Try

### 📲 ALT+TAB To Change Window Focus
* Change Window Focus With ALT+TAB

### 🗺️ Window Location Indicator At Canvas Mode (MAPS)
* Automatically Spawn A Minimaps Every Change Into Canvas Mode

---

### 🚀 Advanced Shell Engine (`fn_spawn`)

* Non-blocking execution
* Direct `/bin/sh` piping
* Fast command dispatch
* Script + system control ready

---

### 🎛️ Native Hardware Controls

Pre-mapped keys for:

* Volume → `pamixer`
* Brightness → `brightnessctl`
* Uses XF86 multimedia keys

---

### 🔊 PipeWire / WirePlumber Ready

* Modern Linux audio stack compatible
* No DE required
* Clean integration

---

## 🛠️ Dependencies

```bash
sudo pacman -S libx11 pamixer brightnessctl base-devel
```

---

## ⚙️ Build & Install

```bash
git clone https://github.com/yourusername/nyxwm.git
cd nyxwm
make clean && make
sudo make install
```

---

## 🔧 Configuration — `config.h`

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
make && sudo make install
```

---

## 🏁 Launch

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

---

## 📜 License

MIT License.

Do whatever you want with it.
Break it if you can.

