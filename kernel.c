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

// ===============================
// SectorZero-ARM: Raspberry Pi 4
// ===============================

// Peripheral Base Address for Raspberry Pi 4
#define PERIPHERAL_BASE 0xFE000000 

// PL011 UART MMIO
#define UART0_DR ((volatile unsigned int *)(PERIPHERAL_BASE + 0x201000))

// Mailbox MMIO
#define MBOX_BASE   (PERIPHERAL_BASE + 0xB880)
#define MBOX_READ   ((volatile unsigned int *)(MBOX_BASE + 0x00))
#define MBOX_STATUS ((volatile unsigned int *)(MBOX_BASE + 0x18))
#define MBOX_WRITE  ((volatile unsigned int *)(MBOX_BASE + 0x20))
#define MBOX_FULL   0x80000000
#define MBOX_EMPTY  0x40000000

// Global State
volatile unsigned int __attribute__((aligned(16))) mbox[36];
unsigned int *global_fb = 0;
int global_pitch = 0;

// Include the 8x8 font array directly
#include "font8x8_basic.h"

// Converts an unsigned integer to a decimal string
void int_to_string(unsigned int val, char *buffer) {
    if (val == 0) {
        buffer[0] = '0'; 
        buffer[1] = '\0'; 
        return;
    }
    
    char temp[16]; 
    int i = 0;
    
    // Extract digits backwards
    while (val > 0) { 
        temp[i++] = (val % 10) + '0'; 
        val /= 10; 
    }
    
    // Reverse them into the final buffer
    int j = 0;
    while (i > 0) { 
        buffer[j++] = temp[--i]; 
    }
    buffer[j] = '\0';
}

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
// HDMI Text Rendering (8x8 Font)
// ==========================================
// Each individual character draw
void draw_char(unsigned char c, int x, int y, unsigned int fg_color, unsigned int bg_color) {
    if (!global_fb) return;
    
    if (c > 127) c = '?'; 
    
    const char *glyph = font8x8_basic[c];
    
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            int is_pixel_set = glyph[row] & (1 << col);
            unsigned int *pixel_addr = (unsigned int *)((unsigned char *)global_fb 
                                       + ((y + row) * global_pitch) 
                                       + ((x + col) * 4));
            
            *pixel_addr = is_pixel_set ? fg_color : bg_color;
        }
    }
}
// Drawing a character string
void draw_string(const char *s, int x, int y, unsigned int fg_color, unsigned int bg_color) {
    int current_x = x;
    int current_y = y;
    
    while (*s != '\0') {
        if (*s == '\n') {
            current_y += 8;  // Move down 8 pixels
            current_x = x;   // Carriage return
        } else if (*s == '\r') {
            current_x = x;
        } else {
            draw_char(*s, current_x, current_y, fg_color, bg_color);
            current_x += 8;  // Advance 8 pixels right
        }
        s++;
    }
}

// ==========================================
// Kernel Entry Point
// ==========================================
void kmain(void) {
    print_uart0("SectorZero-ARM: Booting on Raspberry Pi 4 Silicon!\r\n");
    print_uart0("Requesting Framebuffer from VideoCore GPU...\r\n");

    // ===============================================================
    // Returns the raw 32-bit Board Revision code from the GPU Mailbox
    // ===============================================================
    unsigned int get_board_revision() {
        mbox[0] = 7 * 4;       // Total size of this array in bytes
        mbox[1] = 0;           // Request code
        mbox[2] = 0x10002;     // Tag ID: Get board revision
        mbox[3] = 4;           // Value buffer size
        mbox[4] = 8;           // Request/response indicator
        mbox[5] = 0;           // GPU overwrites this with the Revision Code
        mbox[6] = 0;           // End tag

        if(mailbox_call(8) && mbox[5] != 0) {
            return mbox[5]; 
        }
        return 0;
    }

    // ========================================
    // Returns the ARM memory size in Megabytes
    // ========================================
    unsigned int get_memory_size_mb() {
        mbox[0] = 8 * 4;       // Total size of this array in bytes
        mbox[1] = 0;           // Request code
        mbox[2] = 0x10005;     // Tag ID: Get ARM memory
        mbox[3] = 8;           // Value buffer size
        mbox[4] = 8;           // Request/response indicator
        mbox[5] = 0;           // GPU overwrites this with Base Address
        mbox[6] = 0;           // GPU overwrites this with Size in bytes
        mbox[7] = 0;           // End tag

        if(mailbox_call(8) && mbox[6] != 0) {
            // mbox[6] contains the size in bytes. 
            // Divide by (1024 * 1024) to get Megabytes (MB).
            return mbox[6] / 0x100000; 
        }
        return 0; // Return 0 on failure
    }

    // ===============
    // Get memory size
    // ===============
    unsigned int mem_mb = get_memory_size_mb();
    char mem_str[16];
    int_to_string(mem_mb, mem_str);

    // ============================================
    // Get Board Revision - raw 32-bit code, part 2
    // ============================================
    // Verify Bit 23 is set (meaning it uses the New-Style format)
    unsigned int rev = get_board_revision();
    char *board_str = "Unknown Board";
    if (rev & (1 << 23)) {
        // Shift right by 4 bits and mask out everything except the 8-bit Type code
        unsigned int type = (rev >> 4) & 0xFF;
        switch(type) {
            case 0x11: 
                board_str = "Raspberry Pi 4 Model B";
                break;
            case 0x14:
                board_str = "Raspberry Pi 400";
                break;
        }
    }

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
        // Success: Mask the bus address and cast to a 64-bit safe pointer
        global_fb = (unsigned int *)((unsigned long)(mbox[28] & 0x3FFFFFFF)); 
        global_pitch = mbox[33];
        
        print_uart0("GPU Mailbox success! Drawing to HDMI...\r\n");

        // Clear the screen to a retro dark blue
        unsigned int bg_color = 0xFF0000AA;
        for(int y = 0; y < 768; y++) {
            for(int x = 0; x < 1024; x++) {
                unsigned int *pixel = (unsigned int *)((unsigned char *)global_fb + (y * global_pitch) + (x * 4));
                *pixel = bg_color;
            }
        }

        // Draw our boot text
        unsigned int white = 0xFFFFFFFF;
        
        draw_string("SectorZero-ARM (AArch64)", 16, 16, white, bg_color);
        draw_string("Detected Board: ", 16, 32, white, bg_color);
        draw_string(board_str, 16 + (16 * 8), 32, white, bg_color);
        draw_string("Detected GPU RAM: ", 16, 40, white, bg_color);
        draw_string(mem_str, 16 + (18 * 8), 40, white, bg_color);
        // Print the unit suffix
        int offset = 0;
        while(mem_str[offset] != '\0') offset++; // Find length of the number
        draw_string(" MB (LPDDR2)", 16 + (18 * 8) + (offset * 8), 40, white, bg_color);
        draw_string("Video: 1024x768x32 ARGB Framebuffer OK", 16, 48, white, bg_color);
        
        draw_string(">_", 16, 74, white, bg_color);
        
    } else {
        print_uart0("GPU Mailbox failure. Halting.\r\n");
    }

    // Halt the CPU in a low-power state
    while(1) { asm volatile("wfe"); }
}