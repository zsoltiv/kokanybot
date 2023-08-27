CC ?= cc
CFLAGS ?= -g3 -Og
CFLAGS += -std=c17
LDFLAGS += -li2c -lpthread `pkg-config --libs libgpiod`

BIN = kokanybot

SRCDIR = src
BUILDDIR = build

SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%.o,$(SRC))

.PHONY: clean

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
	install -m 744 $(BIN) /usr/bin/$(BIN)
	install -m 644 kokanybot.service /etc/systemd/system
	install -m 744 kokanystream.sh /usr/bin
	install -m 644 kokanystream.service /etc/systemd/system
	install -m 744 kokanyaudio.sh /usr/bin
	install -m 644 kokanyaudio.service /etc/systemd/system
	systemctl enable kokanybot.service
	systemctl enable kokanystream.service
	systemctl enable kokanyaudio.service
