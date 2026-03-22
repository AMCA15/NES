#pragma once

#include <stdint.h>
#include <stddef.h>

#include "mapper.h"

#define VISIBLE_SCANLINES 240
#define VISIBLE_DOTS 256
#define NTSC_SCANLINES_PER_FRAME 261
#define PAL_SCANLINES_PER_FRAME 311
#define DOTS_PER_SCANLINE 341
#define END_DOT 340


#ifdef USE_RGB565_PIXEL_FORMAT
typedef uint16_t pixel_t;
#else
typedef uint32_t pixel_t;
#endif


enum{
    BG_TABLE        = 1 << 4,
    SPRITE_TABLE    = 1 << 3,
    SHOW_BG_8       = 1 << 1,
    SHOW_SPRITE_8   = 1 << 2,
    SHOW_BG         = 1 << 3,
    SHOW_SPRITE     = 1 << 4,
    LONG_SPRITE     = 1 << 5,
    SPRITE_0_HIT    = 1 << 6,
    FLIP_HORIZONTAL = 1 << 6,
    FLIP_VERTICAL   = 1 << 7,
    V_BLANK         = 1 << 7,
    GENERATE_NMI    = 1 << 7,
    RENDER_ENABLED  = 0x18,
    BASE_NAMETABLE  = 0x3,
    FINE_Y          = 0x7000,
    COARSE_Y        = 0x3E0,
    COARSE_X        = 0x1F,
    VERTICAL_BITS   = 0x7BE0,
    HORIZONTAL_BITS = 0x41F,
    X_SCROLL_BITS   = 0x1f,
    Y_SCROLL_BITS   = 0x73E0
};


typedef struct PPU{
    size_t frames;
    pixel_t *screen;
    uint8_t V_RAM[0x1000];
    uint8_t OAM[256];
    uint8_t OAM_cache[8];
    uint8_t palette[0x20];
    uint8_t OAM_cache_len;
    uint8_t ctrl;
    uint8_t mask;
    uint8_t status;
    uint16_t dots;
    uint16_t scanlines;
    uint16_t scanlines_per_frame;

    uint16_t v;
    uint16_t t;
    uint8_t x;
    uint8_t w;
    uint8_t oam_address;
    uint8_t buffer;

    uint8_t render;
    uint8_t bus;

    struct Emulator* emulator;
    Mapper* mapper;
} PPU;


#ifdef USE_RGB565_PIXEL_FORMAT
// RGB565 palette
static const pixel_t nes_palette[64] = {
    0x632c, 0x0151, 0x1094, 0x3814, 0x5811, 0x680b, 0x6820, 0x50e0,
    0x31a0, 0x0a40, 0x0280, 0x0261, 0x0209, 0x0000, 0x0000, 0x0000,
    0xad75, 0x12fb, 0x421f, 0x713f, 0xa0d9, 0xb0cf, 0xb144, 0x9a60,
    0x6b60, 0x3c00, 0x0c80, 0x0466, 0x03f1, 0x0000, 0x0000, 0x0000,
    0xffff, 0x659f, 0x949f, 0xc3bf, 0xf35f, 0xfb99, 0xfba0, 0xecac,
    0xbe00, 0x8ec0, 0x5f26, 0x4710, 0x4e7b, 0x4a71, 0x0000, 0x0000,
    0xffff, 0xc6ff, 0xd69f, 0xee3f, 0xfe3f, 0xfe1d, 0xfe58, 0xf694,
    0xe712, 0xcf72, 0xbf8d, 0xb799, 0xb75e, 0xbdf7, 0x0000, 0x0000
};
#else
// ARGB8888 palette
static const pixel_t nes_palette[64] = {
    0xff666666, 0xff002a88, 0xff1412a7, 0xff3b00a4, 0xff5c007e, 0xff6e0040, 0xff6c0600, 0xff561d00,
    0xff333500, 0xff0b4800, 0xff005200, 0xff004f08, 0xff00404d, 0xff000000, 0xff000000, 0xff000000,
    0xffadadad, 0xff155fd9, 0xff4240ff, 0xff7527fe, 0xffa01acc, 0xffb71e7b, 0xffb53120, 0xff994e00,
    0xff6b6d00, 0xff388700, 0xff0c9300, 0xff008f32, 0xff007c8d, 0xff000000, 0xff000000, 0xff000000,
    0xfffffeff, 0xff64b0ff, 0xff9290ff, 0xffc676ff, 0xfff36aff, 0xfffe6ecc, 0xfffe8170, 0xffea9e22,
    0xffbcbe00, 0xff88d800, 0xff5ce430, 0xff45e082, 0xff48cdde, 0xff4f4f4f, 0xff000000, 0xff000000,
    0xfffffeff, 0xffc0dfff, 0xffd3d2ff, 0xffe8c8ff, 0xfffbc2ff, 0xfffec4ea, 0xfffeccc5, 0xfff7d8a5,
    0xffe4e594, 0xffcfef96, 0xffbdf4ab, 0xffb3f3cc, 0xffb5ebf2, 0xffb8b8b8, 0xff000000, 0xff000000,
};
#endif


void execute_ppu(PPU* ppu);
void reset_ppu(PPU* ppu);
void exit_ppu(PPU* ppu);
void init_ppu(struct Emulator* emulator);
uint8_t read_status(PPU* ppu);
uint8_t read_ppu(PPU* ppu);
void set_ctrl(PPU* ppu, uint8_t ctrl);
void write_ppu(PPU* ppu, uint8_t value);
void dma(PPU* ppu, uint8_t value);
void set_scroll(PPU* ppu, uint8_t coord);
void set_address(PPU* ppu, uint8_t address);
void set_oam_address(PPU* ppu, uint8_t address);
uint8_t read_oam(PPU* ppu);
void write_oam(PPU* ppu, uint8_t value);
uint8_t read_vram(PPU* ppu, uint16_t address);
void write_vram(PPU* ppu, uint16_t address, uint8_t value);
