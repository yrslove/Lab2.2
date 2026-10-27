CC = gcc
CFLAGS = -std=c11 -O2 -Wall -Wextra -Wpedantic -Werror -Iinclude
BUILD = build

.PHONY: all static shared run run-shared test clean

all: static
static: $(BUILD)/monitor
shared: $(BUILD)/monitor_shared

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/arraylist.o: lib/arraylist.c include/arraylist.h | $(BUILD)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD)/arraylist_pic.o: lib/arraylist.c include/arraylist.h | $(BUILD)
	$(CC) $(CFLAGS) -fPIC -MMD -MP -c $< -o $@

$(BUILD)/monitor.o: src/monitor.c include/monitor.h include/arraylist.h | $(BUILD)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD)/main.o: src/main.c include/monitor.h include/arraylist.h | $(BUILD)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD)/libarraylist.a: $(BUILD)/arraylist.o
	ar rcs $@ $^

$(BUILD)/libarraylist.so: $(BUILD)/arraylist_pic.o
	$(CC) -shared -o $@ $^

$(BUILD)/monitor: $(BUILD)/main.o $(BUILD)/monitor.o $(BUILD)/libarraylist.a
	$(CC) $(CFLAGS) $(BUILD)/main.o $(BUILD)/monitor.o $(BUILD)/libarraylist.a -o $@

$(BUILD)/monitor_shared: $(BUILD)/main.o $(BUILD)/monitor.o $(BUILD)/libarraylist.so
	$(CC) $(CFLAGS) $(BUILD)/main.o $(BUILD)/monitor.o -L$(BUILD) -larraylist -Wl,-rpath,'$$ORIGIN' -o $@

run: static
	./$(BUILD)/monitor -p -v -h -t 0 -n 3

run-shared: shared
	./$(BUILD)/monitor_shared -p -v -h -t 0 -n 3

test: static shared
	sh tests/check.sh

clean:
	rm -rf $(BUILD)

-include $(wildcard $(BUILD)/*.d)
