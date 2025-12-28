#!/usr/bin/env bash
# Helper script to run ESP-IDF commands in DevContainer from host
# Usage: ./scripts/dev.sh build|clean|menuconfig|set-target|shell

set -euo pipefail

CONTAINER_IMAGE="espressif/esp-matter"
WORKSPACE="/workspace"

# Find running DevContainer
find_container() {
    podman ps --filter "ancestor=${CONTAINER_IMAGE}" --format "{{.Names}}" | head -n1
}

# Run command in container
run_in_container() {
    local container
    container=$(find_container)

    if [[ -z "$container" ]]; then
        echo "Error: DevContainer not running"
        echo "Start it via VS Code: Reopen in Container"
        exit 1
    fi

    podman exec -it "$container" bash -c "cd ${WORKSPACE} && source /opt/esp/idf/export.sh > /dev/null 2>&1 && source /opt/esp/esp-matter/export.sh > /dev/null 2>&1 && $*"
}

USB_PORT="${ESP_PORT:-/dev/cu.usbmodem*}"

# Find USB port
find_port() {
    local port
    # shellcheck disable=SC2086
    port=$(ls ${USB_PORT} 2>/dev/null | head -n1)
    if [[ -z "$port" ]]; then
        echo "Error: No USB device found at ${USB_PORT}" >&2
        echo "Set ESP_PORT env var or connect device" >&2
        exit 1
    fi
    echo "$port"
}

case "${1:-help}" in
    build)
        echo "Building in DevContainer..."
        run_in_container "idf.py build"
        ;;
    clean)
        echo "Cleaning build..."
        run_in_container "idf.py fullclean"
        ;;
    menuconfig)
        echo "Opening menuconfig..."
        run_in_container "idf.py menuconfig"
        ;;
    set-target)
        target="${2:-esp32c6}"
        echo "Setting target to ${target}..."
        run_in_container "idf.py set-target ${target}"
        ;;
    reconfigure)
        echo "Reconfiguring project..."
        run_in_container "idf.py reconfigure"
        ;;
    size)
        echo "Showing binary size..."
        run_in_container "idf.py size"
        ;;
    flash)
        port=$(find_port)
        binary="build/smart-garland.bin"
        if [[ ! -f "$binary" ]]; then
            echo "Error: Binary not found. Run './scripts/dev.sh build' first"
            exit 1
        fi
        echo "Flashing to ${port}..."
        espflash flash "$binary" --port "$port"
        ;;
    monitor)
        port=$(find_port)
        echo "Opening monitor on ${port}..."
        espflash monitor --port "$port"
        ;;
    flash-monitor)
        port=$(find_port)
        binary="build/smart-garland.bin"
        if [[ ! -f "$binary" ]]; then
            echo "Error: Binary not found. Run './scripts/dev.sh build' first"
            exit 1
        fi
        echo "Flashing and monitoring on ${port}..."
        espflash flash "$binary" --port "$port" --monitor
        ;;
    shell)
        echo "Opening shell in DevContainer..."
        container=$(find_container)
        if [[ -z "$container" ]]; then
            echo "Error: DevContainer not running"
            exit 1
        fi
        podman exec -it "$container" bash
        ;;
    status)
        container=$(find_container)
        if [[ -n "$container" ]]; then
            echo "DevContainer running: ${container}"
            podman exec "$container" bash -c "source /opt/esp/idf/export.sh > /dev/null 2>&1 && echo IDF version: \$(idf.py --version)"
        else
            echo "DevContainer not running"
        fi
        ;;
    help|--help|-h|*)
        echo "ESP-Matter DevContainer Helper"
        echo ""
        echo "Usage: ./scripts/dev.sh <command>"
        echo ""
        echo "Build commands (run in container):"
        echo "  build         Build the project"
        echo "  clean         Full clean (remove build/)"
        echo "  menuconfig    Open ESP-IDF menuconfig"
        echo "  set-target    Set chip target (default: esp32c6)"
        echo "  reconfigure   Reconfigure CMake"
        echo "  size          Show binary size breakdown"
        echo ""
        echo "Flash commands (run on host, USB required):"
        echo "  flash         Flash binary to device"
        echo "  monitor       Open serial monitor"
        echo "  flash-monitor Flash and open monitor"
        echo ""
        echo "Other:"
        echo "  shell         Open bash in container"
        echo "  status        Check if container is running"
        echo ""
        echo "Environment:"
        echo "  ESP_PORT      USB port (default: /dev/cu.usbmodem*)"
        echo ""
        echo "Examples:"
        echo "  ./scripts/dev.sh set-target esp32c6"
        echo "  ./scripts/dev.sh build"
        echo "  ./scripts/dev.sh flash-monitor"
        echo "  ESP_PORT=/dev/cu.usbmodem2101 ./scripts/dev.sh flash"
        ;;
esac
