CXX = g++
CXXFLAGS = -std=gnu++17 -Wall

SRC := $(wildcard src/*.cpp)
OBJ := $(patsubst %.cpp, %.o, $(SRC))
TARGET = libext.so

.PHONY: clean

%.o : %.cpp
	$(CXX) -o $@ -c $(@:.o=.cpp) -fpic $(CXXFLAGS)

$(TARGET) : $(OBJ)
	$(CXX) -o $@ -shared $(CXXFLAGS) $^

clean :
	find src -name \*.o -delete
	rm -f $(TARGET)
