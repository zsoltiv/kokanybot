CC = cc
STDC = -std=c11
CFLAGS = -g -D_XOPEN_SOURCE=700 -O0
LDFLAGS = -li2c -lpthread `pkg-config --libs opencv4 libgpiod libavcodec libavformat libavutil`

BIN = kokanybot

SRCDIR = src
BUILDDIR = build

SRC := $(wildcard $(SRCDIR)/*.c)
OBJ += $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%.o,$(SRC))

.PHONY: all

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

$(BUILDDIR)/%.c.o: $(SRCDIR)/%.c
	mkdir -p $(BUILDDIR)
	$(CC) $< -c $(STDC) $(CFLAGS) -o $@

clean:
	rm -rf $(BUILDDIR) $(BIN)

install: $(BIN)
	cp $(BIN) /usr/bin/$(BIN)
	cp kokanybot.service /etc/systemd/system

run: all
	./$(BIN)
