// QEMU 'virt' machine PL011 UART Data Register
volatile unsigned int * const UART0_DR = (unsigned int *)0x09000000;

void print_uart0(const char *s) {
    while(*s != '\0') {
        *UART0_DR = (unsigned int)(*s);
        s++;
    }
}

void kmain(void) {
    print_uart0("SectorZero-ARM: AArch64 Bare Metal Booting...\n");
    
    // Halt the system in an infinite loop
    while(1) {
        // We could use inline assembly for 'wfe' (Wait For Event) here
        // to save CPU cycles instead of a tight spinloop.
    }
}