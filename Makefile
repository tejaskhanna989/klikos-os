# KlikOS Build System
# Linux kernel + from-scratch userspace GUI

CC      := gcc
CXX     := g++
CFLAGS  := -Wall -Wextra -O2 -std=c11 -Wno-unused-parameter -Wno-sign-compare -Wno-unused-variable -Wno-unused-function -Wno-error \
           -I src/display -I src/toolkit -I src/shell \
           -I src/framework -I src/lib
LDFLAGS := -lm

# Directories
SRC_DIR     := src
BUILD_DIR   := build
DISPLAY_DIR := src/display
TOOLKIT_DIR := src/toolkit
SHELL_DIR   := src/shell
FRAMEWORK_DIR := src/framework
LIB_DIR     := src/lib
INIT_DIR    := src/init
APPS_DIR    := src/apps

# Kernel build
KERNEL_SRC  := $(SRC_DIR)/kernel
KERNEL_CONFIG := config/kernel_defconfig

# Userspace object files
DISPLAY_OBJ   := $(BUILD_DIR)/display/display.o \
                 $(BUILD_DIR)/display/input.o

TOOLKIT_OBJ   := $(BUILD_DIR)/toolkit/toolkit.o

SHELL_OBJ     := $(BUILD_DIR)/shell/shell.o \
                 $(BUILD_DIR)/shell/window_manager.o

FRAMEWORK_OBJ := $(BUILD_DIR)/framework/framework.o

LIB_OBJ       := $(BUILD_DIR)/lib/filesystem.o \
                 $(BUILD_DIR)/lib/networking.o \
                 $(BUILD_DIR)/lib/power.o \
                 $(BUILD_DIR)/lib/secure_storage.o \
                 $(BUILD_DIR)/lib/update.o \
                 $(BUILD_DIR)/lib/app_store.o

INIT_OBJ      := $(BUILD_DIR)/init/main.o

# All apps
APPS := Settings Files Terminal Browser AppStore WhatsApp Camera Gallery \
        Phone Messages Email Calendar Clock Calculator Notes Reminders \
        Maps Weather Music Podcasts Videos TV Books News Stocks Health \
        Fitness Home Shortcuts Measure Translate VoiceMemos FindMy \
        Contacts FaceTime GameCenter TestFlight Tips Accessibility SystemUpdate

APP_BINARIES := $(foreach app,$(APPS),$(BUILD_DIR)/apps/$(app))

.PHONY: all clean kernel userspace apps run-qemu help

all: userspace apps

# ---------- Kernel ----------
kernel:
	@echo "Building Linux kernel..."
	@make -C $(KERNEL_SRC) -j$$(nproc) 2>/dev/null || \
		(echo "No kernel source found at $(KERNEL_SRC). Download kernel first." && false)

# ---------- Userspace ----------
userspace: $(DISPLAY_OBJ) $(TOOLKIT_OBJ) $(SHELL_OBJ) $(FRAMEWORK_OBJ) $(LIB_OBJ) $(INIT_OBJ) $(BUILD_DIR)/init/klikos

$(BUILD_DIR)/display/display.o: $(DISPLAY_DIR)/display.c $(DISPLAY_DIR)/display.h
	@mkdir -p $(BUILD_DIR)/display
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/display/input.o: $(DISPLAY_DIR)/input.c $(DISPLAY_DIR)/input.h
	@mkdir -p $(BUILD_DIR)/display
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/toolkit/toolkit.o: $(TOOLKIT_DIR)/toolkit.c $(TOOLKIT_DIR)/toolkit.h
	@mkdir -p $(BUILD_DIR)/toolkit
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/shell/shell.o: $(SHELL_DIR)/shell.c $(SHELL_DIR)/shell.h
	@mkdir -p $(BUILD_DIR)/shell
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/shell/window_manager.o: $(SHELL_DIR)/window_manager.c $(SHELL_DIR)/window_manager.h
	@mkdir -p $(BUILD_DIR)/shell
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/framework/framework.o: $(FRAMEWORK_DIR)/framework.c $(FRAMEWORK_DIR)/framework.h
	@mkdir -p $(BUILD_DIR)/framework
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/lib/filesystem.o: $(LIB_DIR)/filesystem.c $(LIB_DIR)/filesystem.h
	@mkdir -p $(BUILD_DIR)/lib
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/lib/networking.o: $(LIB_DIR)/networking.c $(LIB_DIR)/networking.h
	@mkdir -p $(BUILD_DIR)/lib
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/lib/power.o: $(LIB_DIR)/power.c $(LIB_DIR)/power.h
	@mkdir -p $(BUILD_DIR)/lib
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/lib/secure_storage.o: $(LIB_DIR)/secure_storage.c $(LIB_DIR)/secure_storage.h
	@mkdir -p $(BUILD_DIR)/lib
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/lib/update.o: $(LIB_DIR)/update.c $(LIB_DIR)/update.h
	@mkdir -p $(BUILD_DIR)/lib
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/lib/app_store.o: $(LIB_DIR)/app_store.c $(LIB_DIR)/app_store.h
	@mkdir -p $(BUILD_DIR)/lib
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/init/main.o: $(INIT_DIR)/main.c
	@mkdir -p $(BUILD_DIR)/init
	$(CC) $(CFLAGS) -c $< -o $@

# Link init (PID 1)
$(BUILD_DIR)/init/klikos: $(INIT_OBJ) $(DISPLAY_OBJ) $(TOOLKIT_OBJ) $(SHELL_OBJ) $(FRAMEWORK_OBJ) $(LIB_OBJ)
	@mkdir -p $(BUILD_DIR)/init
	$(CC) $(INIT_OBJ) $(DISPLAY_OBJ) $(TOOLKIT_OBJ) $(SHELL_OBJ) $(FRAMEWORK_OBJ) $(LIB_OBJ) -o $@ $(LDFLAGS)

# ---------- Apps ----------
apps: $(APP_BINARIES)

define APP_RULE
$(BUILD_DIR)/apps/$(1): $(APPS_DIR)/$(1).c $(DISPLAY_OBJ) $(TOOLKIT_OBJ) $(SHELL_OBJ) $(FRAMEWORK_OBJ)
	@mkdir -p $(BUILD_DIR)/apps
	$(CC) $(CFLAGS) -DAPPS_NAME=\"$(1)\" $(APPS_DIR)/$(1).c \
	    $(DISPLAY_OBJ) $(TOOLKIT_OBJ) $(SHELL_OBJ) $(FRAMEWORK_OBJ) -o $$@ $(LDFLAGS)
endef

$(foreach app,$(APPS),$(eval $(call APP_RULE,$(app))))

# ---------- Run ----------
run-qemu: userspace apps
	@echo "Running KlikOS in QEMU..."
	qemu-system-x86_64 \
	    -machine q35 \
	    -cpu max \
	    -smp 4 \
	    -m 2048M \
	    -serial stdio \
	    -vga std \
	    -display gtk \
	    -no-reboot \
	    -kernel build/kernel/bzImage 2>/dev/null || \
	    (echo "No kernel found. Build kernel first." && false)

run-qemu-arm: userspace apps
	@echo "Running KlikOS in QEMU (ARM64)..."
	qemu-system-aarch64 \
	    -machine virt \
	    -cpu cortex-a72 \
	    -smp 4 \
	    -m 2048M \
	    -serial stdio \
	    -vga none \
	    -device ramfb \
	    -display gtk \
	    -no-reboot \
	    -kernel build/kernel/Image 2>/dev/null || \
	    (echo "No ARM64 kernel found. Build kernel first." && false)

# ---------- Clean ----------
clean:
	rm -rf $(BUILD_DIR)

# ---------- Help ----------
help:
	@echo "KlikOS Build System"
	@echo "==================="
	@echo "all          - Build userspace + all 40 apps"
	@echo "kernel       - Build Linux kernel"
	@echo "userspace    - Build display server, toolkit, shell, init"
	@echo "apps         - Build all 40 system apps"
	@echo "run-qemu     - Run in QEMU x86_64"
	@echo "run-qemu-arm - Run in QEMU ARM64"
	@echo "clean        - Remove build output"
	@echo "help         - Show this help"