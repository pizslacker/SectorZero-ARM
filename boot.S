; SectorZero-64 - Bare-Metal x86_64 Bootloader & Minimal C Kernel
; Copyright (C) k!M/pizslacker 2026
;
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
