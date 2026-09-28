// GBA hello world: a square you can move with the D-pad.
// No libraries needed. We talk to the GBA hardware directly.

#include <stdint.h>

// --- Hardware addresses (memory-mapped registers) ---
#define REG_DISPCNT  (*(volatile uint16_t *)0x04000000) // display settings
#define REG_VCOUNT   (*(volatile uint16_t *)0x04000006) // which screen row is drawing
#define REG_KEYINPUT (*(volatile uint16_t *)0x04000130) // button state
#define VRAM         ((volatile uint16_t *)0x06000000)  // screen memory

#define MODE3  0x0003 // bitmap mode: every pixel is one 16-bit color
#define BG2_ON 0x0400 // turn on the background layer Mode 3 uses

#define SCREEN_W 240
#define SCREEN_H 160

// --- Buttons ---
#define KEY_RIGHT 0x0010
#define KEY_LEFT  0x0020
#define KEY_UP    0x0040
#define KEY_DOWN  0x0080

// Colors are 5 bits each of red, green, blue (0-31)
#define RGB15(r, g, b) ((r) | ((g) << 5) | ((b) << 10))

#define BG_COLOR     RGB15(2, 2, 8)   // dark blue
#define SQUARE_COLOR RGB15(31, 20, 0) // orange
#define SQUARE_SIZE  16
#define SPEED        2

static void fill_screen(uint16_t color) {
    for (int i = 0; i < SCREEN_W * SCREEN_H; i++) {
        VRAM[i] = color;
    }
}

static void draw_square(int x, int y, uint16_t color) {
    for (int row = 0; row < SQUARE_SIZE; row++) {
        for (int col = 0; col < SQUARE_SIZE; col++) {
            VRAM[(y + row) * SCREEN_W + (x + col)] = color;
        }
    }
}

// Wait until the screen finishes drawing one frame (about 60 times a second)
static void wait_for_vblank(void) {
    while (REG_VCOUNT >= SCREEN_H) {}
    while (REG_VCOUNT < SCREEN_H) {}
}

int main(void) {
    REG_DISPCNT = MODE3 | BG2_ON;

    int x = (SCREEN_W - SQUARE_SIZE) / 2;
    int y = (SCREEN_H - SQUARE_SIZE) / 2;

    fill_screen(BG_COLOR);
    draw_square(x, y, SQUARE_COLOR);

    // The game loop: runs once per frame, forever
    while (1) {
        wait_for_vblank();

        // Buttons read as 0 when pressed, so we flip the bits
        uint16_t keys = ~REG_KEYINPUT & 0x03FF;

        int new_x = x;
        int new_y = y;
        if (keys & KEY_LEFT)  new_x -= SPEED;
        if (keys & KEY_RIGHT) new_x += SPEED;
        if (keys & KEY_UP)    new_y -= SPEED;
        if (keys & KEY_DOWN)  new_y += SPEED;

        // Keep the square on screen
        if (new_x < 0) new_x = 0;
        if (new_y < 0) new_y = 0;
        if (new_x > SCREEN_W - SQUARE_SIZE) new_x = SCREEN_W - SQUARE_SIZE;
        if (new_y > SCREEN_H - SQUARE_SIZE) new_y = SCREEN_H - SQUARE_SIZE;

        // Only redraw if it moved: erase the old square, draw the new one
        if (new_x != x || new_y != y) {
            draw_square(x, y, BG_COLOR);
            x = new_x;
            y = new_y;
            draw_square(x, y, SQUARE_COLOR);
        }
    }
}
