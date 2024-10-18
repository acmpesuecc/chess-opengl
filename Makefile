# Compiler and Flags
CC = gcc
CFLAGS = -Wall -g -DGLEW_STATIC -DGLFW_INCLUDE_NONE

# Directories
SRC_DIR = src
BUILD_DIR = build
RES_DIR = res

# Source Files
SOURCES = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/**/*.c) $(wildcard $(SRC_DIR)/**/**/*.c)
HEADERS = $(wildcard $(SRC_DIR)/*.h) $(wildcard $(SRC_DIR)/**/*.h) $(wildcard $(SRC_DIR)/**/**/*.h)
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SOURCES))

# OS Detection
ifeq ($(OS),Windows_NT)
    OS = windows
    TARGET_EXEC = $(BUILD_DIR)/chess.exe
    LIB = -L./lib -lglew32 -lglfw3 -lcglm -lfreetype -lgdi32 -lopengl32
    INC = -I./include -I./include/freetype
else
    UNAME := $(shell uname -s)
    ifeq ($(UNAME),Darwin)
        OS = macos
        TARGET_EXEC = $(BUILD_DIR)/chess
        LIB = -L/usr/local/lib -lGLEW -lglfw -lGL -lm -lcglm -lfreetype
        INC = -I/usr/local/include -I/usr/local/include/freetype2
    else ifeq ($(UNAME),Linux)
        OS = linux
        TARGET_EXEC = $(BUILD_DIR)/chess
        LIB = -L/usr/lib -lGLEW -lglfw -lGL -lm -lcglm -lfreetype
        INC = -I/usr/include -I/usr/include/freetype2
    else
        $(error OS not supported by this Makefile)
    endif
endif

# Ensure directories exist
dir_guard=@mkdir -p $(@D)

# Default Target
.PHONY: all clean

all: $(TARGET_EXEC)

# Link Object Files into Executable
$(TARGET_EXEC): $(OBJECTS)
	$(dir_guard)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LIB)

# Compile Source Files into Object Files
$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c $(HEADERS)
	$(dir_guard)
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

# Clean Build Directory
clean:
	rm -rf $(BUILD_DIR)

ifeq ($(OS),Windows_NT)
    OS = windows
    CXXFLAGS += -I/path/to/windows/headers
    LDFLAGS += -L/path/to/windows/libs -lwin_specific_lib
else
    UNAME := $(shell uname -s)
    ifeq ($(UNAME),Darwin)
        OS = macos
        CXXFLAGS += -I/path/to/macos/headers
        LDFLAGS += -L/path/to/macos/libs -lGLU -lGL -lm
    else ifeq ($(UNAME),Linux)
        OS = linux
        CXXFLAGS += -I/path/to/linux/headers
        LDFLAGS += -L/path/to/linux/libs -lGLU -lGL -lm
    else
        $(error OS not supported by this Makefile)
    endif
endif

# Common rules and targets
all: main.o
    $(CXX) $(CXXFLAGS) -o myprogram main.o $(LDFLAGS)

main.o: main.cpp
    $(CXX) $(CXXFLAGS) -c main.cpp

clean:
    rm -f *.o myprogram
