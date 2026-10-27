#ifndef MONITOR_H
#define MONITOR_H

#include <stdbool.h>
#include "arraylist.h"
#include <stddef.h>
#include <time.h>

enum {
    MONITOR_PROC = 1,
    MONITOR_VIDEO = 2,
    MONITOR_HARD = 4
};

typedef struct {
    unsigned mask;
    unsigned interval;
    size_t iterations;
} MonitorConfig;

typedef struct {
    time_t timestamp;
    char computer[256];
    char user[256];
    unsigned long uid;
    size_t iteration;
    unsigned mask;
    double proc;
    double video;
    double hard;
} MonitorSample;

void monitor_collect(MonitorSample *sample, unsigned mask, size_t iteration);
bool monitor_save(const char *path, const ArrayList *samples, size_t count);
void monitor_print(const MonitorSample *sample);

#endif
