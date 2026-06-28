CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lX11 -lXi

SRC = main.c wm.c util.c
OBJ = $(SRC:.c=.o)

nyxwm: $(OBJ)
        $(CC) $(CFLAGS) -o nyxwm $(OBJ) $(LDFLAGS)
        @echo "=== Build Success ==="
        @echo "Updating ~/.xinitrc in home directory..."
        @echo '#!/bin/bash' > $$HOME/.xinitrc
        @echo '' >> $$HOME/.xinitrc
        @echo 'eval $$(dbus-launch --sh-syntax)' >> $$HOME/.xinitrc
        @echo 'xwallpaper --stretch ~/Pictures/Wallpaper/wallhaven-6lk75x.jpg' >> $$HOME/.xinitrc
        @echo 'wal -i ~/Pictures/Wallpaper/wallhaven-6lk75x.jpg' >> $$HOME/.xinitrc
        @echo 'picom --config ~/.config/picom/picom.conf &' >> $$HOME/.xinitrc
        @echo '' >> $$HOME/.xinitrc
        @echo 'nyxwm &' >> $$HOME/.xinitrc
        @echo 'exec dbus-run-session sleep infinity' >> $$HOME/.xinitrc
        @chmod +x $$HOME/.xinitrc
        @echo "Restarting nyxwm to the latest version..."
        @pkill nyxwm || true
        @./nyxwm &

%.o: %.c
        $(CC) $(CFLAGS) -c $< -o $@

clean:
        rm -f *.o nyxwm

install: nyxwm
        install -Dm755 nyxwm /usr/local/bin/nyxwm
        @echo "Creating nyxwm-session wrapper..."
        @echo '#!/bin/sh' > nyxwm-session
        @echo 'while true; do /usr/local/bin/nyxwm; sleep 0.5; done' >> nyxwm-session
        install -Dm755 nyxwm-session /usr/local/bin/nyxwm-session
        @rm -f nyxwm-session
        @echo "Creating XSession entry (/usr/share/xsessions/nyxwm.desktop)..."
        @echo '[Desktop Entry]' > nyxwm.desktop
        @echo 'Name=NyxWM' >> nyxwm.desktop
        @echo 'Comment=Nyx Window Manager with Auto-Restart' >> nyxwm.desktop
        @echo 'Exec=startx' >> nyxwm.desktop
        @echo 'Type=Application' >> nyxwm.desktop
        install -Dm644 nyxwm.desktop /usr/share/xsessions/nyxwm.desktop
        @rm -f nyxwm.desktop

.PHONY: clean install
