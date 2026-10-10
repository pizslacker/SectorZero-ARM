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

// Raspberry Pi 4 Peripheral Base
#define PERIPHERAL_BASE 0xFE000000 

// PL011 UART
#define UART0_DR ((volatile unsigned int *)(PERIPHERAL_BASE + 0x201000))

// Mailbox MMIO
#define MBOX_BASE   (PERIPHERAL_BASE + 0xB880)
#define MBOX_READ   ((volatile unsigned int *)(MBOX_BASE + 0x00))
#define MBOX_STATUS ((volatile unsigned int *)(MBOX_BASE + 0x18))
#define MBOX_WRITE  ((volatile unsigned int *)(MBOX_BASE + 0x20))
#define MBOX_FULL   0x80000000
#define MBOX_EMPTY  0x40000000

volatile unsigned int __attribute__((aligned(16))) mbox[36];

void print_uart0(const char *s) {
    while(*s != '\0') {
        *UART0_DR = (unsigned int)(*s);
        s++;
    }
}

int mailbox_call(unsigned char channel) {
    unsigned int r = (((unsigned int)((unsigned long)&mbox) & ~0xF) | (channel & 0xF));
    while(*MBOX_STATUS & MBOX_FULL) { }
    *MBOX_WRITE = r;
    while(1) {
        while(*MBOX_STATUS & MBOX_EMPTY) { }
        if(r == *MBOX_READ) return mbox[1] == 0x80000000;
    }
    return 0;
}

void kmain(void) {
    print_uart0("SectorZero-ARM: Booting on Raspberry Pi 4 Silicon!\r\n");

    // Request a 1024x768 framebuffer
    mbox[0] = 35 * 4; mbox[1] = 0;
    mbox[2] = 0x48003; mbox[3] = 8; mbox[4] = 8; mbox[5] = 1024; mbox[6] = 768; // Physical
    mbox[7] = 0x48004; mbox[8] = 8; mbox[9] = 8; mbox[10] = 1024; mbox[11] = 768; // Virtual
    mbox[12] = 0x48009; mbox[13] = 8; mbox[14] = 8; mbox[15] = 0; mbox[16] = 0; // Offset
    mbox[17] = 0x48005; mbox[18] = 4; mbox[19] = 4; mbox[20] = 32; // Depth (ARGB)
    mbox[21] = 0x48006; mbox[22] = 4; mbox[23] = 4; mbox[24] = 1; // RGB
    mbox[25] = 0x40001; mbox[26] = 8; mbox[27] = 8; mbox[28] = 16; mbox[29] = 0; // Allocate
    mbox[30] = 0x40008; mbox[31] = 4; mbox[32] = 4; mbox[33] = 0; // Pitch
    mbox[34] = 0;

    if(mailbox_call(8) && mbox[20] == 32 && mbox[28] != 0) {
        print_uart0("GPU Mailbox success! Drawing to HDMI...\r\n");
        unsigned int *fb = (unsigned int *)(mbox[28] & 0x3FFFFFFF); 
        int pitch = mbox[33];
        
        for(int y = 0; y < 768; y++) {
            for(int x = 0; x < 1024; x++) {
                unsigned int *pixel = (unsigned int *)((unsigned char *)fb + (y * pitch) + (x * 4));
                *pixel = 0xFF000000 | ((y & 0xFF) << 8) | (x & 0xFF);
            }
        }
    } else {
        print_uart0("GPU Mailbox failure.\r\n");
    }

    while(1) { asm volatile("wfe"); }
}