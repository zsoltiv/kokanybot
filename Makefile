CC ?= cc
CFLAGS = -g -Og
ALLCFLAGS = $(CFLAGS) -std=c11 -D_XOPEN_SOURCE=700
LDFLAGS = -li2c -lpthread `pkg-config --libs libgpiod libudev libinput`

BIN ?= kokanybot

SRCDIR = src
BUILDDIR = build

SRC := $(wildcard $(SRCDIR)/*.c)
OBJ += $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%.o,$(SRC))

.PHONY: all clean

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $^ $(ALLCFLAGS) $(LDFLAGS) -o $@

$(BUILDDIR)/%.c.o: $(SRCDIR)/%.c
	mkdir -p $(BUILDDIR)
	$(CC) $< -c $(ALLCFLAGS) -o $@

clean:
	rm -rf $(BUILDDIR) $(BIN)

install: $(BIN)
	cp $(BIN) /usr/bin/$(BIN)
	cp kokanybot.service /etc/systemd/system

run: all
	./$(BIN)
