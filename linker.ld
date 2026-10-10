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

ENTRY(_start)
SECTIONS
{
    /* QEMU virt machine base RAM address */
    . = 0x40000000;
    
    /* Ensure our boot assembly is at the very beginning of the binary */
    .text.boot : { *(.text.boot) }
    
    .text : { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    
    .bss : {
        . = ALIGN(16);
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    }
}
