CC = cc
CFLAGS = -g
CXXFLAGS = $(CFLAGS)
LDFLAGS = -lgpiod

SRCDIR = src
BUILDDIR = build

SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRC))

# TODO: robot.o-ban van a szerver halozati kodja, ATRAKNI
SERVEROBJ = $(BUILDDIR)/server.o $(BUILDDIR)/gpio.o $(BUILDDIR)/i2c.o
CLIENTOBJ = $(BUILDDIR)/robot.o

all: server client

server: $(SERVEROBJ)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

client: $(SERVEROBJ)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(BUILDDIR)
	$(CC) $< -c $(CFLAGS) -o $@

clean:
	rm -rf $(BUILDDIR) server client
