# SectorZero-ARM

**SectorZero** project from `x86_64` to `ARM` (`AArch64`).

Instead of navigating real-mode to long-mode transitions, SectorZero-ARM boots directly into a flat 64-bit address space (`AArch64`), giving you immediate access to modern C development and Memory-Mapped I/O (`MMIO`).

### Core Architecture & Capabilities
- Multi-Core Boot Management: The boot.S assembly entry point detects the hardware topology using the mpidr_el1 register. It safely parks secondary CPU cores in a low-power wait-for-event (wfe) loop while configuring the stack pointer (sp) for the primary core to execute C code.

- Custom Memory Layout: Driven by a custom linker script (linker.ld), the binary bypasses the standard Linux C library. It maps the code to exact hardware-specific memory addresses (0x40000000 for QEMU virt, 0x80000 for Raspberry Pi) and structures the .bss, .rodata, and .text sections for a freestanding C environment.

- Memory-Mapped I/O (MMIO) Serial Output: In the absence of a legacy VGA text buffer, the kernel communicates directly with the PL011 UART controller (at 0x09000000 on QEMU or 0x3F000000/0xFE000000 on Raspberry Pi) by writing bytes directly to physical memory addresses.

- Mailbox Property Interface (RPi branch): On physical hardware, the kernel speaks directly to the Broadcom VideoCore GPU via a 16-byte aligned message buffer, requesting framebuffer allocations to draw raw ARGB pixels directly to an HDMI monitor.

- Cross-Compiled Flat Binary: Built via a POSIX-compliant Makefile using the aarch64-linux-gnu- toolchain, the project strips away all ELF headers to generate a pure, raw machine-code payload (.bin or kernel8.img) that the emulator or GPU can blindly execute.

## Requirements:
```bash
sudo apt-get install gcc-aarch64-linux-gnu
```

To compile it and immediately launch the QEMU emulator:
```bash
make run
```
(Remember to use Ctrl+A, then X to exit QEMU).


Remove the compiled object files and binaries:
```bash
make clean
```
