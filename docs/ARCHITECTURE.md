# KlikOS Architecture — Full Mobile OS

KlikOS is a complete mobile operating system built from scratch. Nothing here comes from Android, AOSP, or Linux — no Linux kernel, no Bionic libc, no ART/Dalvik, no Android framework.

## Design principles

- **Clean sheet**: every component (bootloader, kernel, libc, init, shell, display server, window manager, app framework, runtime, apps) is written for this project.
- **Mobile first**: touch input, power management, sandboxed apps, secure storage, telephony-style lifecycle.
- **ARM64 first**: primary target `aarch64-klikos-elf` (QEMU virt initially, then real handsets).
- **No Android compatibility**: not an Android replacement in the sense of running Android apps — it's a different OS that serves the same purpose (phone/tablet OS).

## Layer stack

```
+-------------------------------------------------------------+
| Built-in apps: Launcher, Settings, Files, Terminal, ...     |
+-------------------------------------------------------------+
| App framework: Activity, Package, Window, Resource manager  |
+-------------------------------------------------------------+
| App runtime (runs KlikOS ELF apps)                          |
+-------------------------------------------------------------+
| Display server / compositor (custom, Wayland-inspired)      |
+-------------------------------------------------------------+
| Window manager                                              |
+-------------------------------------------------------------+
| Userspace: init (PID 1), shell, daemons, system services    |
+-------------------------------------------------------------+
| KlikOS libc                                                 |
+-------------------------------------------------------------+
| KlikOS kernel:                                              |
|   - scheduler, MM (paging/heap), IPC, syscalls              |
|   - device driver framework                                 |
+-------------------------------------------------------------+
| Bootloader                                                  |
+-------------------------------------------------------------+
| Hardware / QEMU virt (ARM64 aarch64)                        |
+-------------------------------------------------------------+
```

## Components

### Bootloader (`src/bootloader/`)
First code to run on real hardware. For QEMU, the kernel ELF is loaded directly by QEMU `-kernel`, so the bootloader is currently a stub. On real devices it will initialize RAM, load the kernel, and jump to it.

### Kernel (`src/kernel/`)
Runs in EL1. Responsibilities:
- Bootstrap CPU, stack, exception vectors, paging.
- Preemptive scheduler (time-slice round robin).
- GIC interrupt controller + timer driver.
- Physical + virtual memory manager.
- Device driver registration/probing (PL011 UART first).
- Minimal syscall interface: exit, write, read, open, close, yield, sleep, putchar.

### Userspace (`src/userspace/`)
- **libc** — C standard library on top of KlikOS syscalls.
- **init** — PID 1. Mounts filesystems, starts daemons, display server, shell. Reaps orphans.
- **shell** — Command-line shell with builtins (exit, help, echo).
- **coreutils** — Stubs for ls, cat, mount, etc. (wired later).

### Display server (`src/display/server/`)
Custom compositor (not Wayland). Framebuffer output, surface management, input handling (touch/keyboard).

### Window manager (`src/display/wm/`)
Manages application windows/surfaces on the display server.

### App framework (`src/framework/`)
Activity manager, package manager, window manager, resource manager. Clean-sheet — no Android framework code.

### Runtime (`src/runtime/`)
Runs applications compiled to `aarch64-klikos-elf` ELF. Not ART/Dalvik.

### Apps (`src/apps/`)
Built-in apps: Launcher (home screen), Settings, Files, Terminal.

## Boot process

1. QEMU / firmware loads kernel ELF at `0x40080000`, jumps to `_start`.
2. `entry.S`: identify CPU via `MPIDR_EL1`, set up stack, call `kernel_main()`.
3. `kernel_main()`: init PL011 UART, print banner, idle on `wfi`.
4. Future: paging, scheduler, syscalls, launch init (PID 1).
5. `init`: mount rootfs, start daemons, display server, shell, reap loop.

## Syscall numbers

| # | Syscall |
|---|---------|
| 0 | exit |
| 1 | write |
| 2 | read |
| 3 | open |
| 4 | close |
| 5 | yield |
| 6 | sleep |
| 7 | putchar |

## Build system

- `scripts/build-cross-toolchain.sh` — builds `aarch64-klikos-elf` binutils + GCC.
- `scripts/build-os.sh` — builds kernel, userspace, apps, disk image.
- `scripts/build-userspace.sh` — builds libc, init, shell, coreutils.
- `scripts/build-apps.sh` — builds display, framework, runtime, apps.
- `scripts/run-qemu.sh` — builds and runs under QEMU.
- `scripts/install-device.sh` — installs to block device using **sudo** (password: `t`).

## Non-goals

- Running Android apps (no ART/Dalvik, no Bionic, no AOSP).
- Linux binary compatibility (no syscall emulation).
- x86 primary support (ARM64 only).
