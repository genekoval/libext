PROJECT = extensions++
VERSION = 0.1.0
LIBRARY = lib$(PROJECT).so

SRCDIR = src
OBJDIR = obj
INCDIR = include
BINDIR = lib
BUILDS = $(OBJDIR) $(BINDIR)

CC = g++
CCFLAGS = -I $(INCDIR) -std=gnu++17 -Wall

SRC := $(wildcard $(SRCDIR)/*.cpp)
OBJ := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRC))
TARGET = $(BINDIR)/$(LIBRARY).$(VERSION)

$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(CC) -o $@ -c -fpic $(CCFLAGS) $^

$(TARGET) : $(OBJ)
	$(CC) -o $@ -shared $(CCFLAGS) $^

.PHONY: all dir clean version

all : dir $(TARGET)

clean :
	rm -rf $(BUILDS)

dir :
	@mkdir -p $(BUILDS)

version :
	@echo Project: $(PROJECT)
	@echo Version: $(VERSION)
