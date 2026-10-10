/*
 * SectorZero-ARM - Bare-Metal ARM Bootloader & Minimal C Kernel
 * Copyright (C) k!M/pizslacker 2026
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

// Use 0x3F000000 for Raspberry Pi 3
// Use 0xFE000000 for Raspberry Pi 4
#define PERIPHERAL_BASE 0xFE000000 

// PL011 UART Data Register
#define UART0_DR ((volatile unsigned int *)(PERIPHERAL_BASE + 0x201000))

void print_uart0(const char *s) {
    while(*s != '\0') {
        *UART0_DR = (unsigned int)(*s);
        s++;
    }
}

void kmain(void) {
    print_uart0("SectorZero-ARM: Hello from real Raspberry Pi 3 Silicon!\r\n");
    
    while(1) {
        // Halt loop
    }
}
