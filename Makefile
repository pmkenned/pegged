.PHONY: all run clean

CC = gcc
CPPFLAGS =
CFLAGS = -Wall -Wextra -pedantic -O2 -std=c89 -Wno-format
LDFLAGS = 
LDLIBS = 

TARGET := pegged
BUILD_DIR = ./build
SRC = main.c
OBJ = $(SRC:%.c=$(BUILD_DIR)/%.o)
DEP = $(OBJ:%.o=%.d)

include os.mk

ifeq ($(KERNEL),Windows)
	ifeq ($(ENV), CYGWIN_NT)
	endif
	ifeq ($(ENV), MINGW32_NT)
		CXXFLAGS = -Ic:/MinGW/include/ncursesw/
		LDFLAGS = -Lc:/MinGW/lib/
		LDLIBS += -lncursesw
	endif
	TARGET := $(TARGET).exe
endif
ifeq ($(KERNEL),Linux)
		LDLIBS = -lncurses
endif
ifeq ($(KERNEL),Darwin)
		LDLIBS = -lncurses
endif

all: $(BUILD_DIR)/$(TARGET)
ifeq ($(ENV), MINGW32_NT)
ifeq (,$(wildcard $(BUILD_DIR)/*.dll))
	cp c:/MinGW/bin/libgcc_s_dw2-1.dll $(BUILD_DIR)/
	cp c:/MinGW/bin/libncursesw6.dll $(BUILD_DIR)/
endif
endif

$(BUILD_DIR)/$(TARGET): $(OBJ)
	mkdir -p $(BUILD_DIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -c -o $@ $<

-include $(DEP)

run: $(BUILD_DIR)/$(TARGET)
	$(BUILD_DIR)/$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
