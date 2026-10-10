.section ".text.boot"
.global _start

_start:
    // Read the CPU ID from the Multiprocessor Affinity Register
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    // If CPU ID is 0, continue booting. Otherwise, park the core.
    cbz     x1, primary_core

park_core:
    wfe
    b       park_core

primary_core:
    // Set the stack pointer to the start of our code
    // The stack grows downwards in ARM
    ldr     x1, =_start
    mov     sp, x1

    // Branch with Link to our C main function
    bl      kmain

halt:
    b       halt
