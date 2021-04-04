OBJ=$(shell find src -name *.c)
BIN=$(OBJ:src/%.c=build/dui_%)
DEPS=$(OBJ:src/%.c=build/dui_%.d)

all: | build $(BIN)

options:
	@echo $(OBJ)
	@echo $(BIN)

build:
	mkdir build

-include $(DEPS)
build/dui_%: src/%.c Makefile
	gcc -MMD -MP $< -o $@ `pkg-config --cflags --libs gtk+-3.0`

clean:
	rm -r build
