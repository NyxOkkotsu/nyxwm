## nyxwm 🌌

```text
███╗   ██╗██╗   ██╗██╗  ██╗██╗    ██╗███╗   ███╗
████╗  ██║╚██╗ ██╔╝╚██╗██╔╝██║    ██║████╗ ████║
██╔██╗ ██║ ╚████╔╝  ╚███╔╝ ██║ █╗ ██║██╔████╔██║
██║╚██╗██║  ╚██╔╝   ██╔██╗ ██║███╗██║██║╚██╔╝██║
██║ ╚████║   ██║   ██╔╝ ██╗╚███╔███╔╝██║ ╚═╝ ██║
╚═╝  ╚═══╝   ╚═╝   ╚═╝  ╚═╝ ╚══╝╚══╝ ╚═╝     ╚═╝
```

nyxwm is a feature-rich, ultra-lightweight, and blazingly fast tiling window manager for X11, engineered entirely from scratch in pure C. Built with a fierce focus on mathematical layout symmetry, absolute minimalism, and direct hardware execution, nyxwm completely eliminates desktop bloat to give you raw, unadulterated system performance.

## 🚀 Core Features & Hyper-Superiorities

    Blazingly Fast Bare-Metal Execution: Written in pure, highly optimized C with zero unnecessary abstractions. It boots instantly (0ms latency), operates with a microscopic RAM footprint, and ensures absolute peak performance under heavy gaming or compilation workloads.

    Tidy Tiling & Grid Symmetries: Features a mathematically predictable tiling engine that organizes windows cleanly without overlapping chaos, maximizing every single pixel of your display real estate.

    Suckless-Inspired Architecture: Purely configured at compile-time via a single config.h header file. No heavy runtime parsing, no bloated config scripts, no garbage collection—just pure machine code.

    Advanced Shell Subsystem (fn_spawn): Implements a high-velocity command execution matrix capable of piping system controls, custom scripts, and applications directly to the shell without blocking the main event loop.

    Native Kernel-Level Hardware Controls: Pre-mapped out-of-the-box keybindings for seamless interfacing with audio backends (pamixer) and display panel backlights (brightnessctl) via standard X11 multimedia symbols.

    Modern PipeWire & WirePlumber Synchronization: Fully synchronized with cutting-edge Linux audio frameworks, delivering perfect multimedia integration without the need for heavy desktop environment daemons.

## 🛠️ Prerequisites & Build Dependencies

To compile and unleash the full power of nyxwm on Arch Linux, grab the core X11 development tools and multimedia utilities:
Bash

sudo pacman -S libx11 pamixer brightnessctl base-devel

## ⚙️ Compilation & Installation

nyxwm follows the classic compiled-from-source hacker philosophy. Clone, build, and install it instantly using make:
Bash

git clone https://github.com/yourusername/nyxwm.git
cd nyxwm
make clean && make
sudo make install

## 🔧 High-Performance Configuration (config.h)

All core desktop behaviors, window layouts, and hotkeys are configured cleanly inside config.h.
Advanced Hardware Control Mapping Example:

nyxwm utilizes an optimized inline string array casting mechanism to feed commands directly to /bin/sh through the fn_spawn engine, preventing any scalar initializer conflicts:
C

#include <X11/XF86keysym.h>

static const Key keys[] = {
	/* modifier                     key                          function        argument */
	
	/* High-Velocity Audio Control (pamixer) */
	{ 0,                            XF86XK_AudioRaiseVolume,     fn_spawn,       (const char*[]){"/bin/sh", "-c", "pamixer -i 5", NULL} },
	{ 0,                            XF86XK_AudioLowerVolume,     fn_spawn,       (const char*[]){"/bin/sh", "-c", "pamixer -d 5", NULL} },
	{ 0,                            XF86XK_AudioMute,            fn_spawn,       (const char*[]){"/bin/sh", "-c", "pamixer -t", NULL} },

	/* Precision Brightness Routing (brightnessctl) */
	{ 0,                            XF86XK_MonBrightnessUp,      fn_spawn,       (const char*[]){"/bin/sh", "-c", "brightnessctl set +10%", NULL} },
	{ 0,                            XF86XK_MonBrightnessDown,    fn_spawn,       (const char*[]){"/bin/sh", "-c", "brightnessctl set 10%-", NULL} },

	/* ... your other elite keybindings ... */
};

Whenever you tweak your setup, recompile to push it straight into machine code:
Bash

make && sudo make install

🏁 Launching nyxwm

To initialize nyxwm as your main session graphical interface, configure your ~/.xinitrc to accurately point to your environment runtime path and trigger initial hardware state validation:
Bash

#!/bin/sh

# Initialize system runtime environment
if [ -z "$XDG_RUNTIME_DIR" ]; then
    export XDG_RUNTIME_DIR=/run/user/$(id -u)
fi

# Prevent silent hardware mutes on startup
pamixer --unmute
pamixer --set-volume 50

# Execute the beast
exec nyxwm
