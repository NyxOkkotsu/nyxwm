CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lX11 -lXi

SRC = main.c wm.c util.c
OBJ = $(SRC:.c=.o)

nyxwm: $(OBJ)
	$(CC) $(CFLAGS) -o nyxwm $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o nyxwm

install: nyxwm
	install -Dm755 nyxwm /usr/local/bin/nyxwm

.PHONY: clean install