#define _POSIX_C_SOURCE 200809L
#include "monitor.h"
#include "arraylist.h"

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

static double temperature(double minimum, double maximum) {
    return minimum + (maximum - minimum) * ((double)rand() / RAND_MAX);
}

static void format_time(time_t timestamp, char *buffer, size_t size) {
    struct tm value;
    if (localtime_r(&timestamp, &value) == NULL ||
        strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &value) == 0) {
        snprintf(buffer, size, "невідомо");
    }
}

static void csv_field(FILE *file, const char *value) {
    fputc('"', file);
    for (const unsigned char *ptr = (const unsigned char *)value; *ptr; ++ptr) {
        if (*ptr == '"') {
            fputc('"', file);
        }
        fputc(*ptr, file);
    }
    fputc('"', file);
}

void monitor_collect(MonitorSample *sample, unsigned mask, size_t iteration) {
    memset(sample, 0, sizeof(*sample));
    sample->timestamp = time(NULL);
    sample->mask = mask;
    sample->iteration = iteration;
    sample->uid = (unsigned long)getuid();

    if (gethostname(sample->computer, sizeof(sample->computer)) != 0) {
        snprintf(sample->computer, sizeof(sample->computer), "невідомий");
    }
    sample->computer[sizeof(sample->computer) - 1] = '\0';

    struct passwd *account = getpwuid(getuid());
    const char *name = account != NULL ? account->pw_name : getenv("USER");
    snprintf(sample->user, sizeof(sample->user), "%s", name != NULL ? name : "невідомий");

    if (mask & MONITOR_PROC) {
        sample->proc = temperature(30.0, 100.0);
    }
    if (mask & MONITOR_VIDEO) {
        sample->video = temperature(30.0, 120.0);
    }
    if (mask & MONITOR_HARD) {
        sample->hard = temperature(30.0, 50.0);
    }
}

void monitor_print(const MonitorSample *sample) {
    char timestamp[32];
    format_time(sample->timestamp, timestamp, sizeof(timestamp));
    printf("%s | %s | %s | %zu", timestamp, sample->computer, sample->user,
           sample->iteration);
    if (sample->mask & MONITOR_PROC) {
        printf(" | ЦП: %.1f C", sample->proc);
    }
    if (sample->mask & MONITOR_VIDEO) {
        printf(" | Відеокарта: %.1f C", sample->video);
    }
    if (sample->mask & MONITOR_HARD) {
        printf(" | Диск: %.1f C", sample->hard);
    }
    putchar('\n');
}

bool monitor_save(const char *path, const ArrayList *samples, size_t count) {
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        return false;
    }

    bool ok = fputs("Час;Комп'ютер;Користувач;UID;Ітерація;ЦП, C;Відеокарта, C;Диск, C\n", file) >= 0;
    for (size_t index = 0; ok && index < count; ++index) {
        MonitorSample sample;
        char timestamp[32];
        if (!arraylist_get(samples, index, &sample)) {
            ok = false;
            break;
        }
        format_time(sample.timestamp, timestamp, sizeof(timestamp));
        csv_field(file, timestamp);
        fputc(';', file);
        csv_field(file, sample.computer);
        fputc(';', file);
        csv_field(file, sample.user);
        if (fprintf(file, ";%lu;%zu;", sample.uid, sample.iteration) < 0) {
            ok = false;
        }
        if (sample.mask & MONITOR_PROC && fprintf(file, "%.2f", sample.proc) < 0) {
            ok = false;
        }
        fputc(';', file);
        if (sample.mask & MONITOR_VIDEO && fprintf(file, "%.2f", sample.video) < 0) {
            ok = false;
        }
        fputc(';', file);
        if (sample.mask & MONITOR_HARD && fprintf(file, "%.2f", sample.hard) < 0) {
            ok = false;
        }
        if (fputc('\n', file) == EOF) {
            ok = false;
        }
    }
    if (fclose(file) != 0) {
        ok = false;
    }
    return ok;
}
