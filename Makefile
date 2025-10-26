# BlackBox DPU Virtual Platform Makefile
# Compilation configuration for modular architecture

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2
LDFLAGS = 
TARGET = blackbox_dpu

# Source files
SRCS = event_queue.c \
       memory.c \
       zstd_accelerator.c \
       dma_engine.c \
       nvme_controller.c \
       ethernet_mac.c \
       bus_interconnect.c \
       soc_core.c \
       main.c

# Object files
OBJS = $(SRCS:.c=.o)

# Header files (for dependency tracking)
HEADERS = blackbox_common.h \
          event_queue.h \
          memory.h \
          zstd_accelerator.h \
          dma_engine.h \
          nvme_controller.h \
          ethernet_mac.h \
          bus_interconnect.h \
          soc_core.h

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJS)
	@echo "Linking $(TARGET)..."
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)
	@echo "Build complete: $(TARGET)"

# Compile source files
%.o: %.c $(HEADERS)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -f $(OBJS) $(TARGET)
	rm -f nvme_storage.bin cloud_log.bin
	@echo "Clean complete"

# Run the program
run: $(TARGET)
	@echo "Running $(TARGET)..."
	./$(TARGET)

# Run with quiet mode
run-quiet: $(TARGET)
	@echo "Running $(TARGET) in quiet mode..."
	./$(TARGET) -q

# Help target
help:
	@echo "BlackBox DPU Virtual Platform Build System"
	@echo "==========================================="
	@echo ""
	@echo "Available targets:"
	@echo "  all         - Build the complete project (default)"
	@echo "  clean       - Remove all build artifacts"
	@echo "  run         - Build and run with verbose output"
	@echo "  run-quiet   - Build and run with minimal output"
	@echo "  help        - Display this help message"
	@echo ""
	@echo "Module Structure:"
	@echo "  event_queue      - Event-driven simulation engine"
	@echo "  memory           - Memory subsystem model"
	@echo "  zstd_accelerator - Hardware compression accelerator"
	@echo "  dma_engine       - Multi-channel DMA controller"
	@echo "  nvme_controller  - NVMe storage interface"
	@echo "  ethernet_mac     - Ethernet network interface"
	@echo "  bus_interconnect - NoC and bus transactions"
	@echo "  soc_core         - High-level SoC orchestration"
	@echo "  main             - Test suite and demonstration"
	@echo ""

.PHONY: all clean run run-quiet help
