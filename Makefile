PRJ_NAME := tundra

SOURCE_FILES := $(wildcard ./src/*.c)
BUILD_FILES := $(subst .c,.o,$(subst src,build,$(SOURCE_FILES)))

HEADER_PATH := include

CC := gcc
CC_FLAGS := -c -std=c99 -I$(HEADER_PATH)

all: buildFolder $(PRJ_NAME)

$(PRJ_NAME): $(BUILD_FILES)
	$(CC) $^ -o $@

./build/%.o: ./src/%.c ./include/%.h
	$(CC) $< $(CC_FLAGS) -o $@

./build/main.o: ./src/main.c
	$(CC) main.c $(CC_FLAGS) -o main.o

buildFolder:
	@ mkdir -p build

clean:
	@ rm -rf ./build/*.o $(PRJ_NAME) *~
	@ rmdir objects
