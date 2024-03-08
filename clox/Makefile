SRCS = $(wildcard *.c)
HDRS = $(wildcard *.h)

BUILD = build
OBJS = $(SRCS:%.c=$(BUILD)/%.o)
CLOX = $(BUILD)/clox

CC = gcc
CFLAGS = -Wall -Wextra -pedantic

$(CLOX): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS)  $^ -o $@

.PHONY: run
run: $(CLOX)
	$(CLOX)

$(BUILD)/%.o: %.c $(HDRS)
	mkdir -p $(BUILD)
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -rf $(BUILD)/
