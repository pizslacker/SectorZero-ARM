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

// ==========================================
// SectorZero-ARM: Raspberry Pi 3 (BCM2837)
// ==========================================

// Peripheral Base Address for Raspberry Pi 3
#define PERIPHERAL_BASE 0x3F000000 

// 1. PL011 UART MMIO
#define UART0_DR ((volatile unsigned int *)(PERIPHERAL_BASE + 0x201000))

// 2. Mailbox MMIO
#define MBOX_BASE   (PERIPHERAL_BASE + 0xB880)
#define MBOX_READ   ((volatile unsigned int *)(MBOX_BASE + 0x00))
#define MBOX_STATUS ((volatile unsigned int *)(MBOX_BASE + 0x18))
#define MBOX_WRITE  ((volatile unsigned int *)(MBOX_BASE + 0x20))
#define MBOX_FULL   0x80000000
#define MBOX_EMPTY  0x40000000

// 3. Global State
volatile unsigned int __attribute__((aligned(16))) mbox[36];
unsigned int *global_fb = 0;
int global_pitch = 0;

// You must link or include a binary font array here.
#include "font8x16.h"

// ==========================================
// Serial Output
// ==========================================
void print_uart0(const char *s) {
    while(*s != '\0') {
        *UART0_DR = (unsigned int)(*s);
        s++;
    }
}

// ==========================================
// GPU Mailbox Communication
// ==========================================
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

// ==========================================
// HDMI Text Rendering
// ==========================================
void draw_char(unsigned char c, int x, int y, unsigned int fg_color, unsigned int bg_color) {
    if (!global_fb) return;
    
    // ADD 'const' HERE:
    const unsigned char *glyph = &font8x16[c * 16];
    
    for (int row = 0; row < 16; row++) {
        for (int col = 0; col < 8; col++) {
            int is_pixel_set = glyph[row] & (1 << (7 - col));
            unsigned int *pixel_addr = (unsigned int *)((unsigned char *)global_fb 
                                       + ((y + row) * global_pitch) 
                                       + ((x + col) * 4));
            
            *pixel_addr = is_pixel_set ? fg_color : bg_color;
        }
    }
}

void draw_string(const char *s, int x, int y, unsigned int fg_color, unsigned int bg_color) {
    int current_x = x;
    int current_y = y;
    
    while (*s != '\0') {
        if (*s == '\n') {
            current_y += 16;  // Move down 16 pixels
            current_x = x;    // Carriage return
        } else if (*s == '\r') {
            current_x = x;
        } else {
            draw_char(*s, current_x, current_y, fg_color, bg_color);
            current_x += 8;   // Advance 8 pixels right
        }
        s++;
    }
}

// ==========================================
// Kernel Entry Point
// ==========================================
void kmain(void) {
    print_uart0("SectorZero-ARM: Booting on Raspberry Pi 3!\r\n");
    print_uart0("Requesting Framebuffer from VideoCore GPU...\r\n");

    // Construct Mailbox payload for a 1024x768 32-bit ARGB framebuffer
    mbox[0] = 35 * 4; mbox[1] = 0;
    mbox[2] = 0x48003; mbox[3] = 8; mbox[4] = 8; mbox[5] = 1024; mbox[6] = 768; // Physical
    mbox[7] = 0x48004; mbox[8] = 8; mbox[9] = 8; mbox[10] = 1024; mbox[11] = 768; // Virtual
    mbox[12] = 0x48009; mbox[13] = 8; mbox[14] = 8; mbox[15] = 0; mbox[16] = 0; // Offset
    mbox[17] = 0x48005; mbox[18] = 4; mbox[19] = 4; mbox[20] = 32; // Depth
    mbox[21] = 0x48006; mbox[22] = 4; mbox[23] = 4; mbox[24] = 1; // RGB
    mbox[25] = 0x40001; mbox[26] = 8; mbox[27] = 8; mbox[28] = 16; mbox[29] = 0; // Allocate
    mbox[30] = 0x40008; mbox[31] = 4; mbox[32] = 4; mbox[33] = 0; // Pitch
    mbox[34] = 0;

    if(mailbox_call(8) && mbox[20] == 32 && mbox[28] != 0) {
        // Success: Mask the bus address to get the CPU physical address
        global_fb = (unsigned int *)((unsigned long)(mbox[28] & 0x3FFFFFFF));
        global_pitch = mbox[33];
        
        print_uart0("GPU Mailbox success! Drawing to HDMI...\r\n");

        // 1. Clear the screen to a retro dark blue
        unsigned int bg_color = 0xFF0000AA;
        for(int y = 0; y < 768; y++) {
            for(int x = 0; x < 1024; x++) {
                unsigned int *pixel = (unsigned int *)((unsigned char *)global_fb + (y * global_pitch) + (x * 4));
                *pixel = bg_color;
            }
        }

        // 2. Draw our boot text
        unsigned int white = 0xFFFFFFFF;
        
        draw_string("SectorZero-ARM (AArch64)", 16, 16, white, bg_color);
        draw_string("Hardware: Raspberry Pi 3 (BCM2837)", 16, 32, white, bg_color);
        draw_string("Video: 1024x768x32 ARGB Framebuffer OK", 16, 48, white, bg_color);
        
        draw_string(">_", 16, 80, white, bg_color);
        
    } else {
        print_uart0("GPU Mailbox failure. Halting.\r\n");
    }

    // Halt the CPU in a low-power state
    while(1) { asm volatile("wfe"); }
}