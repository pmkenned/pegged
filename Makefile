.PHONY: all run clean

CC=gcc
CPPFLAGS=-Wall -Wextra -pedantic -O2 -std=c89
CXXFLAGS=
LDFLAGS=
LDLIBS=

TARGET=a.out
BUILD_DIR=./build
SRC=main.c
OBJ=$(SRC:%.c=$(BUILD_DIR)/%.o)
DEP=$(OBJ:%.o=%.d)

-include $(DEP)

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJ)
	mkdir -p $(BUILD_DIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CXXFLAGS) -MMD -c -o $@ $<

run: $(BUILD_DIR)/$(TARGET)
	$(BUILD_DIR)/$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
