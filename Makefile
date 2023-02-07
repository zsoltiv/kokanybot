CC = cc
CXX = c++
CXXINC = `pkg-config --cflags opencv4`
CFLAGS = -g
CXXFLAGS = $(CXXINC) $(CFLAGS)
LDFLAGS = -li2c `pkg-config --libs opencv4 libgpiod libudev libinput`

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
	$(CC) $< -c $(CFLAGS) -o $@

$(BUILDDIR)/%.cpp.o: $(SRCDIR)/%.cpp
	mkdir -p $(BUILDDIR)
	$(CXX) $< -c $(CXXFLAGS) -o $@

clean:
	rm -rf $(BUILDDIR) $(BIN)
