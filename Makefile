EXE=moonbase
OS=$(shell uname)
ifeq ($(OS), Linux)
INCLUDE_FLAGS=-I/usr/include/lua5.2 -I/usr/include/SDL2 -I./unzip -I/usr/include/json-c
LINK_FLAGS=-llua5.2 -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lz -lm -ldl -ltalloc -ljson-c
else ifeq ($(OS), Darwin)
INCLUDE_FLAGS=-I./external/include/json -I./external/include/talloc  -I./external/include/lua -I./unzip -F./external/lib/macosx
LINK_FLAGS=-framework SDL2 -framework SDL2_image -framework SDL2_mixer -framework SDL2_ttf -L./external/lib/macosx -ltalloc -ljson -llua -lm -lz -ldl
endif

BUILD_DIR=build

SOURCE=$(shell find . -name '*.c' -exec basename {} \;)
SOURCE_UNZIP=$(shell find ./unzip -name '*.c' -exec basename {} \;)
OBJ=$(patsubst %.c,$(BUILD_DIR)/obj/%.o,$(SOURCE)) $(patsubst %.c,$(BUILD_DIR)/obj/%.o,$(SOURCE_UNZIP))

all: makedirs $(BUILD_DIR)/$(EXE)

$(BUILD_DIR)/obj/%.o : %.c
	$(CC) -g -c $(INCLUDE_FLAGS) -o $@ $<
$(BUILD_DIR)/obj/%.o : unzip/%.c
	$(CC) -g -c $(INCLUDE_FLAGS) -o $@ $<

$(BUILD_DIR)/$(EXE) : $(OBJ)
	$(CC) $(INCLUDE_FLAGS) -g -o $@ $^ $(LINK_FLAGS)

clean:
	rm -rf $(BUILD_DIR)/obj
	rm $(BUILD_DIR)/$(EXE)

makedirs:
	if [ ! -d $(BUILD_DIR)/obj ]; then mkdir -p $(BUILD_DIR)/obj; fi

