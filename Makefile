EXE=moonbase
INCLUDE_FLAGS=-I/usr/include/lua5.2 -I/usr/include/SDL2 -I./unzip
LINK_FLAGS=-llua5.2 -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lz -lm -ldl -ltalloc -ljson-c

BUILD_DIR=build

SOURCE=$(shell find . -name '*.c' -exec basename {} \;)
SOURCE_UNZIP=$(shell find ./unzip -name '*.c' -exec basename {} \;)
OBJ=$(patsubst %.c,$(BUILD_DIR)/obj/%.o,$(SOURCE)) $(patsubst %.c,$(BUILD_DIR)/obj/%.o,$(SOURCE_UNZIP))

all: makedirs $(BUILD_DIR)/$(EXE)

$(BUILD_DIR)/obj/%.o : %.c
	$(CC) -g -c -o $@ $< $(INCLUDE_FLAGS)
$(BUILD_DIR)/obj/%.o : unzip/%.c
	$(CC) -g -c -o $@ $< $(INCLUDE_FLAGS)

$(BUILD_DIR)/$(EXE) : $(OBJ)
	$(CC) $(INCLUDE_FLAGS) -g -o $@ $^ $(LINK_FLAGS)

clean:
	rm -rf $(BUILD_DIR)

makedirs:
	if [ ! -d $(BUILD_DIR)/obj ]; then mkdir -p $(BUILD_DIR)/obj; fi

