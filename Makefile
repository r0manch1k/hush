BUILD_DIR := build

.PHONY: all build run clean rebuild

all: build

build:
	@mkdir -p $(BUILD_DIR)
	@cmake -S . -B $(BUILD_DIR)
	@cmake --build $(BUILD_DIR)

run: build
	@./$(BUILD_DIR)/Hush.app/Contents/MacOS/Hush

clean:
	@rm -rf $(BUILD_DIR)

rebuild: clean build
