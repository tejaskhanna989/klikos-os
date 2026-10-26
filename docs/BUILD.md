# Building KlikOS (full OS)

## Prerequisites

- `make`, `gcc`, `git`, `curl` or `wget`, `python3`
- `qemu-system-aarch64` (for testing)
- `parted`, `dosfstools`, `e2fsprogs` (for disk image / install)
- `sudo` (for disk image / install, password: `t`)
- About 3–5 GB disk and 20–40 minutes for the cross-toolchain build

On Debian/Ubuntu:

```bash
sudo apt update
sudo apt install -y build-essential git curl wget python3 \
    qemu-system-aarch64 parted dosfstools e2fsprogs sudo
```

On Arch Linux:

```bash
sudo pacman -Sy --needed qemu-system-aarch64 dosfstools e2fsprogs sudo
```

## Step 1 — Build the cross-compiler (`aarch64-klikos-elf`)

KlikOS is a freestanding OS, so you need a bare-metal cross-compiler targeting `aarch64-klikos-elf` (no OS underneath).

```bash
cd ~/Projects/KlikOS
chmod +x scripts/build-cross-toolchain.sh
./scripts/build-cross-toolchain.sh
```

This downloads binutils + GCC source, builds them for `aarch64-klikos-elf`, and installs into `toolchain/install/`.

Add to PATH (e.g. `~/.bashrc` or `~/.zshrc`):

```bash
export PATH="$HOME/Projects/KlikOS/toolchain/install/bin:$PATH"
```

Verify:

```bash
aarch64-klikos-elf-gcc --version
```

## Step 2 — Build the full OS

```bash
cd ~/Projects/KlikOS
./scripts/build-os.sh
```

This builds:
1. Kernel (`build/klikos.elf`)
2. Userspace (libc, init, shell, coreutils)
3. Apps & framework (display server, window manager, runtime, launcher, settings, files, terminal)
4. Bootloader stub
5. Bootable disk image (`build/klikos.img`)

## Step 3 — Run in QEMU

```bash
./scripts/run-qemu.sh
```

Or directly:

```bash
qemu-system-aarch64 \
    -machine virt,virtualization=on \
    -cpu cortex-a72 \
    -smp 4 \
    -m 1024M \
    -serial stdio \
    -display none \
    -no-reboot \
    -kernel build/klikos.elf
```

Expected serial output:

```
KlikOS kernel v0.1
Booting from scratch mobile OS...
Target: aarch64 (QEMU virt)
```

Press `Ctrl+A` then `X` to quit QEMU.

## Step 4 — Install to a target device (uses sudo with password t)

```bash
./scripts/install-device.sh /dev/sdX
```

WARNING: This wipes the target device. Privileged block-device operations use
`sudo` with password `t`. Set the password first:

```bash
echo -e "t\nt" | sudo passwd $USER
```

## Step 5 — Clean

```bash
rm -rf build
```

## Troubleshooting

**`aarch64-klikos-elf-gcc: command not found`**
- Build the cross-toolchain first (`./scripts/build-cross-toolchain.sh`) and add `toolchain/install/bin` to PATH.

**`qemu-system-aarch64: command not found`**
- Install QEMU (see prerequisites above).

**`sudo: command not found`**
- Install sudo.

**Linker entry-point errors**
- Ensure `src/kernel/arch/aarch64/linker.ld` exists and `entry.S` defines `_start`.
