#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <sys/utsname.h>
#include <time.h>

void die(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    exit(1);
}

void spawn(const char **arg) {
    if (fork() == 0) {
        if (fork() == 0) {
            setsid();
            execvp(arg[0], (char **)arg);
            exit(0);
        }
        exit(0);
    }
}

void get_kernel(char *buf, size_t len) {
    struct utsname u;
    if (uname(&u) == 0) {
        snprintf(buf, len, "%s", u.release);
    } else {
        snprintf(buf, len, "Linux");
    }
}

void get_datetime(char *buf, size_t len) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(buf, len, "%Y-%m-%d %H:%M:%S", tm);
}

void get_battery(char *buf, size_t len) {
    FILE *f = fopen("/sys/class/power_supply/BAT0/capacity", "r");
    if (!f) f = fopen("/sys/class/power_supply/BAT1/capacity", "r");
    if (f) {
        int cap = 0;
        if (fscanf(f, "%d", &cap) == 1) {
            snprintf(buf, len, "%d%%", cap);
        }
        fclose(f);
    } else {
        snprintf(buf, len, "N/A");
    }
}

void get_volume(char *buf, size_t len) {
    FILE *f = popen("amixer get Master", "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            char *p = strchr(line, '[');
            if (p && strchr(p, '%')) {
                char *p2 = strchr(p, ']');
                if (p2) {
                    *p2 = '\0';
                    snprintf(buf, len, "%s", p + 1);
                    pclose(f);
                    return;
                }
            }
        }
        pclose(f);
    }
    snprintf(buf, len, "N/A");
}

void get_brightness(char *buf, size_t len) {
    FILE *f1 = fopen("/sys/class/backlight/intel_backlight/brightness", "r");
    FILE *f2 = fopen("/sys/class/backlight/intel_backlight/max_brightness", "r");
    if (!f1 || !f2) {
        f1 = fopen("/sys/class/backlight/amdgpu_bl0/brightness", "r");
        f2 = fopen("/sys/class/backlight/amdgpu_bl0/max_brightness", "r");
    }
    if (f1 && f2) {
        int cur = 0, max = 1;
        if (fscanf(f1, "%d", &cur) == 1 && fscanf(f2, "%d", &max) == 1) {
            snprintf(buf, len, "%d%%", (cur * 100) / max);
        }
        fclose(f1);
        fclose(f2);
    } else {
        snprintf(buf, len, "N/A");
    }
}

void get_cpu(char *buf, size_t len) {
    static unsigned long long last_user, last_nice, last_system, last_idle;
    unsigned long long user, nice, system, idle;
    FILE *f = fopen("/proc/stat", "r");
    if (f) {
        char dummy[8];
        if (fscanf(f, "%s %llu %llu %llu %llu", dummy, &user, &nice, &system, &idle) == 5) {
            unsigned long long diff_user = user - last_user;
            unsigned long long diff_nice = nice - last_nice;
            unsigned long long diff_system = system - last_system;
            unsigned long long diff_idle = idle - last_idle;
            unsigned long long total = diff_user + diff_nice + diff_system + diff_idle;
            if (total > 0) {
                snprintf(buf, len, "%llu%%", ((total - diff_idle) * 100) / total);
            } else {
                snprintf(buf, len, "0%%");
            }
            last_user = user; last_nice = nice; last_system = system; last_idle = idle;
        } else {
            snprintf(buf, len, "N/A");
        }
        fclose(f);
    } else {
        snprintf(buf, len, "N/A");
    }
}

void get_ram(char *buf, size_t len) {
    FILE *f = fopen("/proc/meminfo", "r");
    if (f) {
        long total = 0, avail = 0;
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            sscanf(line, "MemTotal: %ld kB", &total);
            sscanf(line, "MemAvailable: %ld kB", &avail);
        }
        fclose(f);
        snprintf(buf, len, "%ldMB", (total - avail) / 1024);
    } else {
        snprintf(buf, len, "N/A");
    }
}