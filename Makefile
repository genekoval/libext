PROJECT = extensions++
VERSION = 0.1.0
NAMESPACE = nova/ext

BUILD = .
PREFIX = $(DEVROOT)

LIBRARY = lib$(PROJECT).so
LIBFILE = $(LIBRARY).$(VERSION)

SRCDIR = src
OBJDIR = obj
INCDIR = include
LIBDIR = lib

CC = g++
CCFLAGS = -I $(INCDIR) -std=gnu++17 -Wall

SRC := $(wildcard $(SRCDIR)/*.cpp)
OBJ := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRC))
TARGET = $(LIBDIR)/$(LIBFILE)
HEADERS := $(wildcard $(INCDIR)/*.h)
HINSTALL = $(INCDIR)/$(NAMESPACE)

$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(CC) -o $@ -c -fpic $(CCFLAGS) $^

$(TARGET) : $(OBJ)
	$(CC) -o $@ -shared $(CCFLAGS) $^

.PHONY: all clean dir install version

all : dir $(TARGET)

clean :
	rm -rf $(OBJDIR) $(LIBDIR)

dir :
	@mkdir -p $(OBJDIR) $(LIBDIR)

install :
	@mkdir -p $(PREFIX)/{$(HINSTALL),$(LIBDIR)}
	@echo COPY header files
	@cp -v $(HEADERS) $(PREFIX)/$(HINSTALL)
	@echo INSTALL $(PREFIX)/$(TARGET)
	@cp $(TARGET) $(PREFIX)/$(LIBDIR)
	@cd $(PREFIX)/$(LIBDIR) && ln -sf $(LIBFILE) $(LIBRARY)

version :
	@echo Project: $(PROJECT)
	@echo Version: $(VERSION)
