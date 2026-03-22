#include <string.h>
#include "ppu.h"
#include "emulator.h"
#include <stdlib.h>
#include "utils.h"
#include "mmu.h"
#include "cpu6502.h"

static void update_NMI(PPU* ppu);
static size_t screen_size;

// Pre-render buffer for sprites on the current scanline
static uint8_t sprite_pixel_buffer[VISIBLE_DOTS];
static uint8_t sprite_priority_buffer[VISIBLE_DOTS];
static uint8_t sprite_zero_buffer[VISIBLE_DOTS];

void init_ppu(struct Emulator* emulator) {
    PPU* ppu = &emulator->ppu;
#if NAMETABLE_MODE
    screen_size = sizeof(pixel_t) * VISIBLE_SCANLINES * VISIBLE_DOTS * 4;
#else
    screen_size = sizeof(pixel_t) * VISIBLE_SCANLINES * VISIBLE_DOTS;
#endif
    ppu->screen = malloc(screen_size);
    ppu->emulator = emulator;
    ppu->mapper = &emulator->mapper;
    ppu->scanlines_per_frame = emulator->type == NTSC ? NTSC_SCANLINES_PER_FRAME : PAL_SCANLINES_PER_FRAME;

    memset(ppu->palette, 0, sizeof(ppu->palette));
    memset(ppu->OAM_cache, 0, sizeof(ppu->OAM_cache));
    memset(ppu->V_RAM, 0, sizeof(ppu->V_RAM));
    memset(ppu->OAM, 0, sizeof(ppu->OAM));
    ppu->oam_address = 0;
    ppu->v = 0;
    reset_ppu(ppu);
}

void reset_ppu(PPU* ppu){
    ppu->t = ppu->x = ppu->dots = 0;
    ppu->scanlines = 261;
    ppu->w = 1;
    ppu->ctrl &= ~0xFC;
    ppu->mask = 0;
    ppu->status = 0;
    ppu->frames = 0;
    ppu->OAM_cache_len = 0;
    memset(ppu->OAM_cache, 0, 8);
    memset(ppu->screen, 0, screen_size);
}

void exit_ppu(PPU* ppu) {
    if(ppu->screen != NULL) {
        free(ppu->screen);
    }
}

void set_address(PPU* ppu, uint8_t address){
    if(ppu->w){
        // first write
        ppu->t &= 0xff;
        ppu->t |= (address & 0x3f) << 8; // store only upto bit 14
        ppu->w = 0;
    }else{
        // second write
        ppu->t &= 0xff00;
        ppu->t |= address;
        ppu->v = ppu->t;
        ppu->w = 1;
    }
}


void set_oam_address(PPU* ppu, uint8_t address){
    ppu->oam_address = address;
}

uint8_t read_oam(PPU* ppu){
    return ppu->OAM[ppu->oam_address];
}

void write_oam(PPU* ppu, uint8_t value){
    ppu->OAM[ppu->oam_address++] = value;
}

void set_scroll(PPU* ppu, uint8_t coord){
    if(ppu->w){
        // first write
        ppu->t &= ~X_SCROLL_BITS;
        ppu->t |= (coord >> 3) & X_SCROLL_BITS;
        ppu->x = coord & 0x7;
        ppu->w = 0;
    }else{
        // second write
        ppu->t &= ~Y_SCROLL_BITS;
        ppu->t |= ((coord & 0x7) << 12) | ((coord & 0xF8) << 2);
        ppu->w = 1;
    }
}

uint8_t read_ppu(PPU* ppu){
    uint8_t prev_buff = ppu->buffer, data;
    ppu->buffer = read_vram(ppu, ppu->v);

    if(ppu->v >= 0x3F00) {
        data = ppu->buffer;
        // read underlying nametable mirrors into buffer
        // 0x3f00 - 0x3fff maps to 0x2f00 - 0x2fff
        ppu->buffer = read_vram(ppu, ppu->v & 0xefff);
    }else
        data = prev_buff;
    ppu->v += ((ppu->ctrl & BIT_2) ? 32 : 1);
    return data;
}

void write_ppu(PPU* ppu, uint8_t value){
    write_vram(ppu, ppu->v, value);
    ppu->v += ((ppu->ctrl & BIT_2) ? 32 : 1);
}

void dma(PPU* ppu, uint8_t address){
    Memory* memory = &ppu->emulator->mem;
    uint8_t* ptr = get_ptr(memory, address * 0x100);
    // halt CPU for DMA and skip extra cycle if on odd cycle
    do_DMA(&ppu->emulator->cpu, 513 + ppu->emulator->cpu.odd_cycle);
    if(ptr == NULL) {
        // Probably in PRG ROM so it is not possible to resolve a pointer
        // due to bank switching, so we do it the slow hard way
        for(int i = 0; i < 256; i++) {
            ppu->OAM[(ppu->oam_address + i) & 0xff] = read_mem(memory, address * 0x100 + i);
        }
    }else {
        // copy from OAM address to the end (256 bytes)
        memcpy(ppu->OAM + ppu->oam_address, ptr, 256 - ppu->oam_address);
        if(ppu->oam_address) {
            // wrap around and copy from start to OAM address if OAM is not 0x00
            memcpy(ppu->OAM, ptr + (256 - ppu->oam_address), ppu->oam_address);
        }
        // last value
        memory->bus = ptr[255];
    }
}



uint8_t read_vram(PPU* ppu, uint16_t address){
    address = address & 0x3fff;

    if(address < 0x2000) {
        ppu->bus = ppu->mapper->read_CHR(ppu->mapper, address);
        return ppu->bus;
    }

    if(address < 0x3F00){
        address = (address & 0xefff) - 0x2000;
        ppu->bus = ppu->V_RAM[ppu->mapper->name_table_map[address / 0x400] + (address & 0x3ff)];
        return ppu->bus;
    }

    if(address < 0x4000)
        // palette RAM provide first 6 bits and remaining 2 bits are open bus
        return ppu->palette[(address - 0x3F00) % 0x20] & 0x3f | (ppu->bus & 0xc0);

    return 0;
}

void write_vram(PPU* ppu, uint16_t address, uint8_t value){
    address = address & 0x3fff;
    ppu->bus = value;

    if(address < 0x2000)
        ppu->mapper->write_CHR(ppu->mapper, address, value);
    else if(address < 0x3F00){
        address = (address & 0xefff) - 0x2000;
        ppu->V_RAM[ppu->mapper->name_table_map[address / 0x400] + (address & 0x3ff)] = value;
    }

    else if(address < 0x4000) {
        address = (address - 0x3F00) % 0x20;
        if(address % 4 == 0) {
            ppu->palette[address] = value;
            ppu->palette[address ^ 0x10] = value;
        }
        else
            ppu->palette[address] = value;
    }

}

uint8_t read_status(PPU* ppu){
    uint8_t status = ppu->status;
    ppu->w = 1;
    ppu->status &= ~BIT_7; // reset v_blank
    update_NMI(ppu);
    return status;
}

void set_ctrl(PPU* ppu, uint8_t ctrl){
    ppu->ctrl = ctrl;
    update_NMI(ppu);
    // set name table in temp address
    ppu->t &= ~0xc00;
    ppu->t |= (ctrl & BASE_NAMETABLE) << 10;
}

static void update_NMI(PPU* ppu) {
    if(ppu->ctrl & BIT_7 && ppu->status & BIT_7)
        interrupt(&ppu->emulator->cpu, NMI);
    else
        interrupt_clear(&ppu->emulator->cpu, NMI);
}

// Optimized Scanline Rendering Function
static void render_scanline(PPU* ppu) {
    uint16_t y = ppu->scanlines;
    pixel_t* screen_ptr = &ppu->screen[y * VISIBLE_DOTS];

    // 1. Sprite Evaluation & Pre-rendering for the scanline
    memset(sprite_pixel_buffer, 0, VISIBLE_DOTS);
    if (ppu->mask & SHOW_SPRITE) {
        memset(sprite_priority_buffer, 0, VISIBLE_DOTS);
        memset(sprite_zero_buffer, 0, VISIBLE_DOTS);
        int height = (ppu->ctrl & LONG_SPRITE) ? 16 : 8;
        int count = 0;
        
        for (int i = 0; i < 64 && count < 8; i++) {
            int tile_y = ppu->OAM[i * 4] + 1;
            int diff = y - tile_y;
            if (diff >= 0 && diff < height) {
                count++;
                uint8_t tile_idx = ppu->OAM[i * 4 + 1];
                uint8_t attr = ppu->OAM[i * 4 + 2];
                uint8_t tile_x = ppu->OAM[i * 4 + 3];
                int y_off = (attr & FLIP_VERTICAL) ? (height - 1 - diff) : diff;

                uint16_t addr;
                if (ppu->ctrl & LONG_SPRITE) {
                    y_off = (y_off & 7) | ((y_off & 8) << 1);
                    addr = ((tile_idx >> 1) * 32) + y_off + ((tile_idx & 1) << 12);
                } else {
                    addr = (tile_idx * 16) + y_off + ((ppu->ctrl & SPRITE_TABLE) ? 0x1000 : 0);
                }

                uint8_t p0 = read_vram(ppu, addr);
                uint8_t p1 = read_vram(ppu, addr + 8);

                for (int x_off = 0; x_off < 8; x_off++) {
                    int px = tile_x + x_off;
                    if (px >= VISIBLE_DOTS) break;
                    if (!(ppu->mask & SHOW_SPRITE_8) && px < 8) continue;

                    uint8_t bit = (attr & FLIP_HORIZONTAL) ? x_off : (7 - x_off);
                    uint8_t color = ((p0 >> bit) & 1) | (((p1 >> bit) & 1) << 1);
                    
                    if (color != 0 && sprite_pixel_buffer[px] == 0) {
                        sprite_pixel_buffer[px] = ppu->palette[0x10 | (color | ((attr & 0x03) << 2))];
                        sprite_priority_buffer[px] = (attr & BIT_5);
                        if (i == 0) sprite_zero_buffer[px] = 1;
                    }
                }
            }
        }
    }

    // 2. Background Rendering and Multiplexing
    uint16_t v_copy = ppu->v;
    for (int x = 0; x < VISIBLE_DOTS; x++) {
        uint8_t bg_palette_idx = 0;
        if (ppu->mask & SHOW_BG && (x >= 8 || (ppu->mask & SHOW_BG_8))) {
            uint16_t tile_addr = 0x2000 | (v_copy & 0x0FFF);
            uint8_t tile = read_vram(ppu, tile_addr);
            uint16_t pattern_addr = (tile * 16 + ((v_copy >> 12) & 0x7)) | ((ppu->ctrl & BG_TABLE) << 8);
            
            uint8_t fine_x = (ppu->x + x) & 0x07;
            bg_palette_idx = (read_vram(ppu, pattern_addr) >> (7 - fine_x)) & 1;
            bg_palette_idx |= ((read_vram(ppu, pattern_addr + 8) >> (7 - fine_x)) & 1) << 1;

            if (bg_palette_idx != 0) {
                uint16_t attr_addr = 0x23C0 | (v_copy & 0x0C00) | ((v_copy >> 4) & 0x38) | ((v_copy >> 2) & 0x07);
                uint8_t attr = read_vram(ppu, attr_addr);
                bg_palette_idx |= ((attr >> (((v_copy >> 4) & 4) | (v_copy & 2))) & 0x03) << 2;
            }
        }

        // Multiplexing logic
        uint8_t final_color_idx;
        uint8_t sp_pixel = sprite_pixel_buffer[x];

        if (sp_pixel != 0) {
            if (bg_palette_idx == 0 || !sprite_priority_buffer[x]) {
                final_color_idx = sp_pixel;
            } else {
                final_color_idx = ppu->palette[bg_palette_idx];
            }
            // Sprite 0 Hit check
            if (sprite_zero_buffer[x] && bg_palette_idx != 0 && x < 255) {
                ppu->status |= SPRITE_0_HIT;
            }
        } else {
            final_color_idx = ppu->palette[bg_palette_idx];
        }

        screen_ptr[x] = nes_palette[final_color_idx & 0x3F];

        // Horizontal scroll increment
        if (((ppu->x + x) & 7) == 7) {
            if ((v_copy & COARSE_X) == 31) {
                v_copy &= ~COARSE_X;
                v_copy ^= 0x400;
            } else {
                v_copy++;
            }
        }
    }
}

void execute_ppu(PPU* ppu) {
    if (ppu->scanlines < VISIBLE_SCANLINES) {
        if (ppu->dots == 0) {
            render_scanline(ppu);
        } else if (ppu->dots == VISIBLE_DOTS + 1 && (ppu->mask & RENDER_ENABLED)) {
            // Vertical increment
            if ((ppu->v & FINE_Y) != FINE_Y) {
                ppu->v += 0x1000;
            } else {
                ppu->v &= ~FINE_Y;
                uint16_t coarse_y = (ppu->v & COARSE_Y) >> 5;
                if (coarse_y == 29) {
                    coarse_y = 0;
                    ppu->v ^= 0x800;
                } else if (coarse_y == 31) {
                    coarse_y = 0;
                } else {
                    coarse_y++;
                }
                ppu->v = (ppu->v & ~COARSE_Y) | (coarse_y << 5);
            }
        } else if (ppu->dots == VISIBLE_DOTS + 2 && (ppu->mask & RENDER_ENABLED)) {
            ppu->v &= ~HORIZONTAL_BITS;
            ppu->v |= ppu->t & HORIZONTAL_BITS;
        } else if (ppu->dots == VISIBLE_DOTS + 4 && ppu->mask & RENDER_ENABLED) {
            ppu->mapper->on_scanline(ppu->mapper);
        }
    } else if (ppu->scanlines == VISIBLE_SCANLINES + 1 && ppu->dots == 1) {
        ppu->status |= V_BLANK;
        update_NMI(ppu);
    } else if (ppu->scanlines == ppu->scanlines_per_frame) {
        if (ppu->dots == 1) {
            ppu->status &= ~(V_BLANK | SPRITE_0_HIT);
            update_NMI(ppu);
        } else if (ppu->dots == VISIBLE_DOTS + 2 && (ppu->mask & RENDER_ENABLED)) {
            ppu->v &= ~HORIZONTAL_BITS;
            ppu->v |= ppu->t & HORIZONTAL_BITS;
        } else if (ppu->dots >= 280 && ppu->dots <= 304 && (ppu->mask & RENDER_ENABLED)) {
            ppu->v &= ~VERTICAL_BITS;
            ppu->v |= ppu->t & VERTICAL_BITS;
        }
    }

    if (++ppu->dots >= DOTS_PER_SCANLINE) {
        ppu->dots = 0;
        if (++ppu->scanlines > ppu->scanlines_per_frame) {
            ppu->scanlines = 0;
            ppu->frames++;
            ppu->render = 1;
        }
    }
}