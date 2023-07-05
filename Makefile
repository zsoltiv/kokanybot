CC = cc
STDC = -std=c17
CFLAGS = -g -D_XOPEN_SOURCE=700 -O0
LDFLAGS = -li2c -lpthread `pkg-config --libs libgpiod`

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

install: all
	systemctl stop kokanybot.service
	systemctl stop kokanystream.service
	systemctl stop kokanyaudio.service
	cp $(BIN) /usr/bin/$(BIN)
	cp kokanybot.service /etc/systemd/system
	cp kokanystream.sh /usr/bin
	cp kokanystream.service /etc/systemd/system
	cp kokanyaudio.sh /usr/bin
	cp kokanyaudio.service /etc/systemd/system
	systemctl enable kokanybot.service
	systemctl enable kokanystream.service
	systemctl enable kokanyaudio.service
