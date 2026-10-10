# SectorZero-ARM (`RPi4` version)

**SectorZero-ARM**: A bare-metal ARM (AArch64) bootloader and freestanding 64-bit C kernel implementation built from scratch without external libraries or standard runtimes.

---

`ARM` processors / System-on-Chips boot differently than f.ex. `x86`/`x86_64`.

Instead of navigating real-mode to long-mode transitions, SectorZero-ARM boots directly into a flat 64-bit address space (`AArch64`), giving you immediate access to modern C development and Memory-Mapped I/O (`MMIO`).

Unlike the legacy x86 BIOS/UEFI standard we're used to, the `ARM` boot sequence is heavily tied to the specific `System-on-Chip` (`SoC`) hardware and the `ARM Trusted Firmware` (`ATF`) standard.

Because `ARM` processors are used in everything from embedded microcontrollers to secure smartphones and servers, the boot process is highly staged to establish a "Chain of Trust" between secure and non-secure environments.

1. BL1: Boot ROM (The Immutable Root)

    - Execution Level: EL3 (Highest Security)

    - On Power-On Reset (POR), the CPU wakes up and begins executing code physically baked into the silicon (the Boot ROM).

    - Purpose: It initializes minimal hardware (just enough CPU cache to act as temporary RAM), verifies the cryptographic signature of the next stage, and loads BL2 from flash memory or eMMC into the SRAM.

2. BL2: Trusted Boot Firmware

    - Execution Level: Secure EL1

    - Purpose: BL2 initializes the main external DRAM. Because it runs in the secure world, it is responsible for loading the remaining firmware components into memory, verifying their signatures, and setting up the translation tables. It then passes control to BL31.

3. BL31: EL3 Runtime Firmware (Secure Monitor)

    - Execution Level: EL3

    - Purpose: This stage stays resident in memory even after the OS boots. It acts as the "Secure Monitor," handling power management (PSCI calls) and managing the context switches between the Secure World (TrustZone) and the Normal World.

4. BL32: Secure-EL1 Payload (Optional)

    - Execution Level: Secure EL1

    - Purpose: If the system uses TrustZone, BL32 is the Secure OS (like OP-TEE). It handles cryptographic operations, DRM, or biometric data isolated from the main Linux kernel.

5. BL33: Non-Trusted Firmware (The Handoff)

    - Execution Level: EL2 (Hypervisor) or EL1 (OS Kernel)

    - Purpose: This is the traditional bootloader in the "Normal World." Common examples include U-Boot, EDK2 (UEFI), or even a custom bare-metal payload like your SectorZero-ARM. BL33 configures the hardware for the non-secure OS, loads the Linux kernel (and Device Tree), and jumps to it.

### Why It Matters for Bare Metal
When you boot your `kernel8.img` on a Raspberry Pi, you are essentially providing the `BL33` payload. The Broadcom GPU on the Pi acts as a `pseudo-BL1/BL2`, reading the SD card, initializing the RAM, and loading your code.

By the time your `boot.S` assembly executes, the processor has already transitioned out of `EL3`, parked the `TrustZone` environment, and dropped your code into the non-secure world to take over.

### Core Architecture & Capabilities
- Multi-Core Boot Management: The boot.S assembly entry point detects the hardware topology using the mpidr_el1 register. It safely parks secondary CPU cores in a low-power wait-for-event (wfe) loop while configuring the stack pointer (sp) for the primary core to execute C code.

- Custom Memory Layout: Driven by a custom linker script (linker.ld), the binary bypasses the standard Linux C library. It maps the code to exact hardware-specific memory addresses (0x40000000 for QEMU virt, 0x80000 for Raspberry Pi) and structures the .bss, .rodata, and .text sections for a freestanding C environment.

- Memory-Mapped I/O (MMIO) Serial Output: In the absence of a legacy VGA text buffer, the kernel communicates directly with the PL011 UART controller (at 0x09000000 on QEMU or 0x3F000000/0xFE000000 on Raspberry Pi) by writing bytes directly to physical memory addresses.

- Mailbox Property Interface (RPi branch): On physical hardware, the kernel speaks directly to the Broadcom VideoCore GPU via a 16-byte aligned message buffer, requesting framebuffer allocations to draw raw ARGB pixels directly to an HDMI monitor.

- Cross-Compiled Flat Binary: Built via a POSIX-compliant Makefile using the aarch64-linux-gnu- toolchain, the project strips away all ELF headers to generate a pure, raw machine-code payload (.bin or kernel8.img) that the emulator or GPU can blindly execute.

## Requirements:
```bash
sudo apt-get install build-essentials gcc-aarch64-linux-gnu qemu-system-arm
```

## Howto Build:
Run:
```bash
make
```

This will build the binary Raspberry Pi 4 `kernel8.img` needed to boot.

### Pi 4 SD Card Reminder
Ensure your SD card has the Pi 4 specific firmware files on the FAT32 root:
- `bootcode.bin`
- `start4.elf`
- `fixup4.dat`
- `bcm2711-rpi-4-b.dtb`
- 
Also ensure your SD card has the Pi specific config file + kernel image on the FAT32 root:
- `config.txt`
- `kernel8.img`

Remove the compiled object files and binaries:
```bash
make clean
```

Copyright (C) k!M/pizslacker 2026
