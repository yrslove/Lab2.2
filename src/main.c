#define _POSIX_C_SOURCE 200809L
#include "arraylist.h"
#include "monitor.h"

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

static void usage(const char *name) {
    printf("Використання: %s [-p] [-v] [-h] [-t секунди] [-n кількість]\n", name);
    puts("  -p, --proc     температура процесора");
    puts("  -v, --video    температура відеокарти");
    puts("  -h, --hard     температура диска");
    puts("  -t, --time     інтервал у секундах (типово 1)");
    puts("  -n, --number   кількість вимірів (типово 5)");
    puts("      --help     довідка");
    puts("  FILE           шлях до вихідного CSV (типово statistics.csv)");
}

static bool parse_number(const char *text, unsigned long upper, unsigned long *number) {
    if (text == NULL || *text == '\0' || *text == '-') {
        return false;
    }
    char *end = NULL;
    errno = 0;
    unsigned long parsed = strtoul(text, &end, 10);
    if (errno != 0 || *end != '\0' || parsed > upper) {
        return false;
    }
    *number = parsed;
    return true;
}

static bool options(int argc, char **argv, MonitorConfig *config) {
    const struct option options_list[] = {
        {"proc", no_argument, NULL, 'p'},
        {"video", no_argument, NULL, 'v'},
        {"hard", no_argument, NULL, 'h'},
        {"time", required_argument, NULL, 't'},
        {"number", required_argument, NULL, 'n'},
        {"help", no_argument, NULL, 1000},
        {NULL, 0, NULL, 0}
    };

    config->mask = 0;
    config->interval = 1;
    config->iterations = 5;
    int option;
    opterr = 0;
    while ((option = getopt_long(argc, argv, "pvht:n:", options_list, NULL)) != -1) {
        unsigned long value;
        switch (option) {
            case 'p': config->mask |= MONITOR_PROC; break;
            case 'v': config->mask |= MONITOR_VIDEO; break;
            case 'h': config->mask |= MONITOR_HARD; break;
            case 't':
                if (!parse_number(optarg, 86400, &value)) return false;
                config->interval = (unsigned)value;
                break;
            case 'n':
                if (!parse_number(optarg, 100000, &value) || value == 0) return false;
                config->iterations = (size_t)value;
                break;
            case 1000:
                usage(argv[0]);
                exit(EXIT_SUCCESS);
            default: return false;
        }
    }
    if (optind < argc) {
        return false;
    }
    if (config->mask == 0) {
        config->mask = MONITOR_PROC | MONITOR_VIDEO | MONITOR_HARD;
    }
    return true;
}

int main(int argc, char **argv) {
    MonitorConfig config;
    if (!options(argc, argv, &config)) {
        fputs("Помилка параметрів командного рядка.\n", stderr);
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    const char *file = getenv("FILE");
    if (file == NULL || *file == '\0') {
        file = "statistics.csv";
    }

    srand((unsigned)(time(NULL) ^ (time_t)getpid()));
    ArrayList *samples = arraylist_create(sizeof(MonitorSample));
    if (samples == NULL) {
        fputs("Недостатньо пам'яті.\n", stderr);
        return EXIT_FAILURE;
    }

    for (size_t index = 0; index < config.iterations; ++index) {
        if (index > 0 && config.interval > 0) {
            struct timespec interval = {(time_t)config.interval, 0};
            while (nanosleep(&interval, &interval) != 0) {
                if (errno != EINTR) {
                    fputs("Помилка очікування.\n", stderr);
                    arraylist_destroy(samples);
                    return EXIT_FAILURE;
                }
            }
        }
        MonitorSample sample;
        monitor_collect(&sample, config.mask, index + 1);
        if (!arraylist_add(samples, &sample)) {
            fputs("Не вдалося зберегти вимір у пам'яті.\n", stderr);
            arraylist_destroy(samples);
            return EXIT_FAILURE;
        }
        monitor_print(&sample);
    }

    bool success = monitor_save(file, samples, arraylist_size(samples));
    arraylist_destroy(samples);
    if (!success) {
        fprintf(stderr, "Не вдалося записати файл: %s\n", file);
        return EXIT_FAILURE;
    }
    printf("Збережено записів: %zu. Файл: %s\n", config.iterations, file);
    return EXIT_SUCCESS;
}
