# KlikOS Boot Process

KlikOS boots on an ARM64 QEMU virt machine. Later stages target real handsets.

## Stage 0 — QEMU / firmware

QEMU loads the kernel ELF at physical address `0x40080000` and jumps to the entry point `_start`. No separate bootloader is used yet under QEMU (QEMU acts as the loader).

## Stage 1 — Assembly entry (`src/kernel/arch/aarch64/entry.S`)

1. Read `MPIDR_EL1` to identify the current CPU.
2. Only CPU 0 (boot processor) continues; other CPUs park in a `wfe` loop.
3. Set up stack pointer (`sp`) pointing at a reserved stack in `.bss.stack`.
4. Branch to C function `kernel_main()`.

## Stage 2 — C kernel (`src/kernel/main.c`)

`kernel_main()`:

1. Initializes the PL011 UART driver at `0x09000000`.
2. Prints boot banner to serial console.
3. Idles executing `wfi`.

## Stage 3 — Userspace (planned)

Once paging + syscalls are implemented:

1. Kernel loads `/init` (userspace init.elf, PID 1).
2. `init` mounts rootfs, starts daemons.
3. `init` starts the display server and window manager.
4. `init` starts the mobile shell (launcher).
5. `init` reaps orphaned processes.

## Memory map (early, planned)

| Region | Purpose |
|--------|---------|
| `0x40080000` | Kernel load address (`_start`, `.text`) |
| After kernel | `.data`, `.bss`, stack |
| `0x09000000` | PL011 UART (serial console) |
| `0x80000000+` | Userspace programs (init, shell, apps) |
| High memory | RAM (up to 1024 MB in QEMU) |

## Future boot stages

- Exception vector setup (sync exceptions, IRQ, FIQ, SError).
- Paging: identity map early → proper virtual memory map.
- Heap / physical page allocator.
- GICv2/v3 + timer init.
- Preemptive scheduler → launch userspace init.
- Display output → mobile shell.
