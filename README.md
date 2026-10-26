# KlikOS — From-Scratch Mobile OS (Linux Kernel + Custom GUI)

KlikOS is a complete mobile operating system built on the **Linux kernel** with a **fully from-scratch userspace GUI**.
No GTK, no Qt, no X11, no Wayland — everything from the display server to the widget toolkit to the apps is written here.

## Architecture

```
+-------------------------------------------------------------+
| 40 System Apps (custom toolkit)                             |
+-------------------------------------------------------------+
| KlikOS Toolkit (buttons, labels, windows, layouts, theming) |
+-------------------------------------------------------------+
| KlikOS Display Server (DRM/KMS or fbdev direct rendering)   |
+-------------------------------------------------------------+
| KlikOS Window Manager / Compositor                          |
+-------------------------------------------------------------+
| KlikOS Init (PID 1) + System Services                       |
+-------------------------------------------------------------+
| Linux Kernel (drivers, filesystem, networking, memory)      |
+-------------------------------------------------------------+
```

## Project Layout

```
KlikOS/
├── README.md           # This file
├── LICENSE             # MIT
├── .gitignore
├── docs/               # Design docs (ARCHITECTURE, BUILD, BOOT_PROCESS)
├── src/                # All source code
│   ├── kernel/         # Linux kernel defconfig + patches
│   ├── drivers/        # Custom drivers (framebuffer, input, touch)
│   ├── display/        # Display server (DRM/KMS + fbdev)
│   ├── toolkit/        # GUI widget toolkit (no GTK/Qt)
│   ├── shell/          # Window manager + springboard (home screen)
│   ├── init/           # PID 1 init system
│   ├── lib/            # C library wrappers + utilities
│   ├── apps/           # 40 system apps
│   └── packages/       # Third-party source for build
├── config/             # Kernel defconfig, GRUB config
├── scripts/            # Build, install, run helpers (uses sudo, password: t)
└── build/              # Build output (gitignored)
```

## Quick Start

```bash
cd ~/Projects/KlikOS

# 1. Build userspace (display server, toolkit, shell, init, 40 apps)
make all

# 2. Build Linux kernel (downloads + compiles v6.6)
./scripts/build-kernel.sh

# 3. Build bootable ISO
./scripts/build-iso.sh

# 4. Run in QEMU with graphical GUI
./scripts/run-qemu.sh
```

## 40 System Apps

Settings, Files, Terminal, Browser, App Store, WhatsApp, Camera, Gallery,
Phone, Messages, Email, Calendar, Clock, Calculator, Notes, Reminders, Maps,
Weather, Music, Podcasts, Videos, TV, Books, News, Stocks, Health, Fitness,
Home, Shortcuts, Measure, Translate, Voice Memos, Find My, Contacts, FaceTime,
Game Center, TestFlight, Tips, Accessibility, System Update.

## GUI Design (From Scratch, No GTK/Qt)

- **Display Server**: Direct DRM/KMS or fbdev rendering with custom framebuffer
- **Widget Toolkit**: Custom-drawn buttons, labels, text fields, sliders, toggles
- **Window Manager**: Custom compositing window manager with home screen
- **Theming**: iOS-inspired design system with rounded corners, vibrant colors
- **Input**: Touchscreen, keyboard, mouse event handling
- **Boot Screen**: Lock screen with swipe-to-unlock, home screen with app grid

## Build Scripts

| Script | Purpose |
|--------|---------|
| `scripts/build-kernel.sh` | Downloads + builds Linux kernel v6.6 |
| `scripts/build-userspace.sh` | Builds display server, toolkit, shell, init |
| `scripts/build-iso.sh` | Creates bootable GRUB ISO |
| `scripts/build-apps.sh` | Builds all 40 system apps |
| `scripts/run-qemu.sh [ARCH]` | Runs under QEMU (x86_64 or aarch64) |
| `scripts/install-device.sh` | Installs to block device using `sudo` (password: `t`) |
| `scripts/clean.sh` | Removes build output |

## Make Targets

| Target | Description |
|--------|-------------|
| `make all` | Build userspace + all 40 apps |
| `make userspace` | Build init, display server, toolkit, shell |
| `make apps` | Build all 40 system apps |
| `make clean` | Remove build output |

## Boot Process

1. Linux kernel boots (v6.6 with KlikOS defconfig)
2. Initramfs loads KlikOS userspace (`/init` = `build/init/klikos`)
3. Display server initializes (DRM/KMS or fbdev)
4. Shell launches lock screen → home screen with 40 app icons
5. Apps launch as fullscreen windows with custom toolkit UI

## Contributing

This is an educational clean-sheet mobile OS. The kernel is Linux, but all
userspace (GUI, toolkit, shell, apps) is written from scratch. No GTK, no Qt,
no X11, no Wayland. Pull requests and issues welcome.
