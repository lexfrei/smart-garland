# Smart Garland Build System
# Uses containerized ESP-IDF for reproducible builds

CONTAINER_IMAGE := smart-garland-builder
CONTAINER_RUNTIME := podman
TARGET ?= esp32c6
PORT ?= $(shell ls /dev/cu.usbmodem* 2>/dev/null | head -n1)

BUILD_DIR := build
BINARY := $(BUILD_DIR)/smart-garland.bin

.PHONY: all build flash monitor flash-monitor clean fullclean container shell menuconfig size help

all: build

# Build container image
container:
	@echo "Building container image..."
	$(CONTAINER_RUNTIME) build --tag $(CONTAINER_IMAGE) --file Containerfile .

# Build firmware (runs in container)
build: container
	@echo "Building for $(TARGET)..."
	$(CONTAINER_RUNTIME) run --rm \
		--volume .:/workspace:Z \
		--workdir /workspace \
		$(CONTAINER_IMAGE) \
		bash -c "source /opt/esp/idf/export.sh > /dev/null 2>&1 && \
		         source /opt/esp/esp-matter/export.sh > /dev/null 2>&1 && \
		         idf.py set-target $(TARGET) && \
		         idf.py build"
	@echo "Build complete: $(BINARY)"

# Flash to device (runs on host)
flash: $(BINARY)
ifndef PORT
	$(error No USB device found. Set PORT=/dev/cu.usbmodemXXX)
endif
	espflash flash $(BINARY) --port $(PORT)

# Serial monitor (runs on host)
monitor:
ifndef PORT
	$(error No USB device found. Set PORT=/dev/cu.usbmodemXXX)
endif
	espflash monitor --port $(PORT)

# Flash and monitor
flash-monitor: $(BINARY)
ifndef PORT
	$(error No USB device found. Set PORT=/dev/cu.usbmodemXXX)
endif
	espflash flash $(BINARY) --port $(PORT) --monitor

# Interactive shell in container
shell: container
	$(CONTAINER_RUNTIME) run --rm -it \
		--volume .:/workspace:Z \
		--workdir /workspace \
		$(CONTAINER_IMAGE) \
		bash

# Interactive menuconfig
menuconfig: container
	$(CONTAINER_RUNTIME) run --rm -it \
		--volume .:/workspace:Z \
		--workdir /workspace \
		$(CONTAINER_IMAGE) \
		bash -c "source /opt/esp/idf/export.sh > /dev/null 2>&1 && \
		         source /opt/esp/esp-matter/export.sh > /dev/null 2>&1 && \
		         idf.py menuconfig"

# Show binary size
size: container
	$(CONTAINER_RUNTIME) run --rm \
		--volume .:/workspace:Z \
		--workdir /workspace \
		$(CONTAINER_IMAGE) \
		bash -c "source /opt/esp/idf/export.sh > /dev/null 2>&1 && \
		         source /opt/esp/esp-matter/export.sh > /dev/null 2>&1 && \
		         idf.py size"

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) sdkconfig

# Full clean including managed components
fullclean: clean
	rm -rf managed_components

help:
	@echo "Smart Garland Build System"
	@echo ""
	@echo "Usage: make [target] [TARGET=esp32c6|esp32h2] [PORT=/dev/cu.usbmodemXXX]"
	@echo ""
	@echo "Build targets:"
	@echo "  build       Build firmware (default)"
	@echo "  clean       Remove build artifacts"
	@echo "  fullclean   Remove build + managed components"
	@echo "  container   Build container image only"
	@echo ""
	@echo "Flash targets:"
	@echo "  flash       Flash firmware to device"
	@echo "  monitor     Open serial monitor"
	@echo "  flash-monitor  Flash and open monitor"
	@echo ""
	@echo "Dev targets:"
	@echo "  shell       Open shell in container"
	@echo "  menuconfig  Open ESP-IDF menuconfig"
	@echo "  size        Show binary size breakdown"
	@echo ""
	@echo "Examples:"
	@echo "  make                     # Build for esp32c6"
	@echo "  make TARGET=esp32h2      # Build for esp32h2"
	@echo "  make flash-monitor       # Flash and monitor"
	@echo "  make PORT=/dev/cu.usbmodem2101 flash"
