CC ?= cc
CFLAGS ?= -g3 -Og
ALLCFLAGS = $(CFLAGS) -std=c17
LDFLAGS += -lpthread `pkg-config --libs libgpiod`

BIN = kokanybot

SRCDIR = src
BUILDDIR = build

SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%.o,$(SRC))

.PHONY: clean

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $^ $(ALLCFLAGS) $(LDFLAGS) -o $@

$(BUILDDIR)/%.c.o: $(SRCDIR)/%.c
	mkdir -p $(BUILDDIR)
	$(CC) $< -c $(ALLCFLAGS) -o $@

clean:
	rm -rf $(BUILDDIR) $(BIN)

install: all
	install -m 744 $(BIN) /usr/bin/$(BIN)
	install -m 644 kokanybot.service /etc/systemd/system
	install -m 744 kokanystream-front.sh /usr/bin
	install -m 644 kokanystream-front.service /etc/systemd/system
	install -m 744 kokanystream-rear.sh /usr/bin
	install -m 644 kokanystream-rear.service /etc/systemd/system
	install -m 744 kokanyaudio.sh /usr/bin
	install -m 644 kokanyaudio.service /etc/systemd/system
	install -m 660 rules/60-camera.rules /etc/udev/rules.d/60-camera.rules
	systemctl enable kokanybot.service
	systemctl enable kokanystream-front.service
	systemctl enable kokanystream-rear.service
	systemctl enable kokanyaudio.service
	systemctl restart kokanybot.service
	systemctl restart kokanystream-front.service
	systemctl restart kokanystream-rear.service
	systemctl restart kokanyaudio.service
