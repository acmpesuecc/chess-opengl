CC = gcc
CFLAGS = -Wall -g -DGLEW_STATIC -DGLFW_INCLUDE_NONE

SRC_DIR = src
BUILD_DIR = build
RES_DIR = res


SOURCES = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/**/*.c) $(wildcard $(SRC_DIR)/**/**/*.c)
HEADERS = $(wildcard $(SRC_DIR)/*.h) $(wildcard $(SRC_DIR)/**/*.h) $(wildcard $(SRC_DIR)/**/**/*.h)
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SOURCES))

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

dir_guard=@mkdir -p $(@D)


.PHONY: all clean

all: $(TARGET_EXEC)

$(TARGET_EXEC): $(OBJECTS)
	$(dir_guard)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LIB)

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c $(HEADERS)
	$(dir_guard)
	$(CC) $(CFLAGS) $(INC) -c $< -o $@


clean:
	rm -rf $(BUILD_DIR)
