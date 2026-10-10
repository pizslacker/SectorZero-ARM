# SectorZero-ARM

**SectorZero** project from `x86_64` to `ARM` (`aarch64`)

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
