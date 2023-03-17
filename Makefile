CC = cc
CXX = c++
CXXINC = `pkg-config --cflags opencv4`
STDC = -std=c11
CFLAGS = -g -O0
CXXFLAGS = $(CXXINC) $(CFLAGS)
LDFLAGS = -li2c -lpthread `pkg-config --libs opencv4 libgpiod libudev libinput`

BIN = kokanybot

SRCDIR = src
BUILDDIR = build

SRC := $(wildcard $(SRCDIR)/*.c)
SRC += $(wildcard $(SRCDIR)/*.cpp)
OBJ += $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%.o,$(SRC))

.PHONY: all

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $^ $(CXXFLAGS) $(LDFLAGS) -o $@

$(BUILDDIR)/%.c.o: $(SRCDIR)/%.c
	mkdir -p $(BUILDDIR)
	$(CC) $< -c $(STDC) $(CFLAGS) -o $@

$(BUILDDIR)/%.cpp.o: $(SRCDIR)/%.cpp
	mkdir -p $(BUILDDIR)
	$(CXX) $< -c $(CXXFLAGS) -o $@

clean:
	rm -rf $(BUILDDIR) $(BIN)

install: $(BIN)
	cp $(BIN) /usr/bin/$(BIN)
