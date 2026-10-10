# SectorZero-ARM

**SectorZero** project from `x86_64` to `ARM` (`AArch64`).

Instead of navigating real-mode to long-mode transitions, SectorZero-ARM boots directly into a flat 64-bit address space (`AArch64`), giving you immediate access to modern C development and Memory-Mapped I/O (`MMIO`).

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
