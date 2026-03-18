#include "emulator.h"
#include "controller.h"
#include <stdlib.h>
#include <string.h>
#include "mapper.h"
#include "debugtools.h"
#include "utils.h"
#include "platform/events.h"
#include "platform/timers.h"

static uint64_t PERIOD;
static uint16_t TURBO_SKIP;

void init_emulator(struct Emulator* emulator, int argc, char *argv[]){
    if(argc < 2) {
        LOG(ERROR, "Input file not provided");
        quit(EXIT_FAILURE);
    }

    char* genie = NULL;
    if(argc == 3 || argc == 6)
        genie = argv[argc - 1];

    memset(emulator, 0, sizeof(Emulator));
    load_file(argv[1], genie, &emulator->mapper);
    emulator->type = emulator->mapper.type;
    emulator->mapper.emulator = emulator;
    if(emulator->type == PAL) {
        PERIOD = 1000000000 / PAL_FRAME_RATE;
        TURBO_SKIP = PAL_FRAME_RATE / PAL_TURBO_RATE;
    }else{
        PERIOD = 1000000000 / NTSC_FRAME_RATE;
        TURBO_SKIP = NTSC_FRAME_RATE / NTSC_TURBO_RATE;
    }

    GraphicsContext* g_ctx = &emulator->g_ctx;

    g_ctx->screen_width = -1;
    g_ctx->screen_height = -1;
    g_ctx->is_tv = 0;

    g_ctx->width = 256;
    g_ctx->height = 240;
    g_ctx->scale = 2;

#if NAMETABLE_MODE
    g_ctx->width = 512;
    g_ctx->height = 480;
    g_ctx->scale = 1;
    if(emulator->mapper.is_nsf) {
        LOG(ERROR, "Can't run NSF Player in Nametable mode");
        quit(EXIT_FAILURE);
    }
    LOG(DEBUG, "RENDERING IN NAMETABLE MODE");
#endif
    graphics_get_api()->init(g_ctx);

    init_mem(emulator);
    init_ppu(emulator);
    init_cpu(emulator);
    init_APU(emulator);

    emulator->exit = 0;
    emulator->pause = 0;
}


void run_emulator(struct Emulator* emulator){
    struct JoyPad* joy1 = &emulator->mem.joy1;
    struct JoyPad* joy2 = &emulator->mem.joy2;
    struct PPU* ppu = &emulator->ppu;
    struct c6502* cpu = &emulator->cpu;
    struct APU* apu = &emulator->apu;
    GraphicsContext* g_ctx = &emulator->g_ctx;
    EmulatorEvent e;
    uint64_t frame_start_ns = timers_get_monotonic_ns();

    while (!emulator->exit) {
#if PROFILE
        if(PROFILE_STOP_FRAME && ppu->frames >= PROFILE_STOP_FRAME)
            break;
#endif
        uint64_t frame_loop_start_ns = timers_get_monotonic_ns();
        while (input_poll_event(&e)) {
            update_joypad(joy1, &e);
            update_joypad(joy2, &e);
            if((joy1->status & RESET_BUTTONS) == RESET_BUTTONS || (joy2->status & RESET_BUTTONS) == RESET_BUTTONS) {
                reset_emulator(emulator);
            }

            if(e.type == EE_EXIT) {
                emulator->exit = 1;
                LOG(DEBUG, "Exiting emulator session (exit event)");
            } else if(e.type == EE_PAUSE) {
                emulator->pause ^= 1;
            } else if(e.type == EE_RESET) {
                reset_emulator(emulator);
            }
        }

        // trigger turbo events
        if(ppu->frames % TURBO_SKIP == 0) {
            turbo_trigger(joy1);
            turbo_trigger(joy2);
        }

        if(!emulator->pause){
            // if ppu.render is set a frame is complete
            if(emulator->type == NTSC) {
                while (!ppu->render) {
                    execute_ppu(ppu);
                    execute_ppu(ppu);
                    execute_ppu(ppu);
                    execute(cpu);
                    execute_apu(apu);
                }
            }else{
                // PAL
                uint8_t check = 0;
                while (!ppu->render) {
                    execute_ppu(ppu);
                    execute_ppu(ppu);
                    execute_ppu(ppu);
                    check++;
                    if(check == 5) {
                        // on the fifth run execute an extra ppu clock
                        // this produces 3.2 scanlines per cpu clock
                        execute_ppu(ppu);
                        check = 0;
                    }
                    execute(cpu);
                    execute_apu(apu);
                }
            }
#if NAMETABLE_MODE
            render_name_tables(ppu, ppu->screen);
#endif
            graphics_get_api()->render(g_ctx, ppu->screen);
            ppu->render = 0;
            queue_audio(apu, g_ctx);
            uint64_t frame_loop_end_ns = timers_get_monotonic_ns();
            uint64_t frame_loop_elapsed_ns = frame_loop_end_ns - frame_loop_start_ns;
            if (frame_loop_elapsed_ns < PERIOD) {
                timers_sleep_us((PERIOD - frame_loop_elapsed_ns) / 1000);
            }
        }else{
            timers_sleep_us(IDLE_SLEEP * 1000);
        }
    }

    uint64_t frame_end_ns = timers_get_monotonic_ns();
    emulator->time_diff = (double)(frame_end_ns - frame_start_ns) / 1000000.0;  // Convert ns to ms
}

void reset_emulator(Emulator* emulator) {
    // Use as exit procedure for TV mode
    if(emulator->g_ctx.is_tv){
        emulator->exit = 1;
        LOG(DEBUG, "Exiting emulator session (reset on TV)");
        return;
    }
    LOG(INFO, "Resetting emulator");
    reset_cpu(&emulator->cpu);
    reset_APU(&emulator->apu);
    reset_ppu(&emulator->ppu);
    if(emulator->mapper.reset != NULL) {
        emulator->mapper.reset(&emulator->mapper);
    }
}

void free_emulator(struct Emulator* emulator){
    LOG(DEBUG, "Starting emulator clean up");
    exit_APU();
    exit_ppu(&emulator->ppu);
    free_mapper(&emulator->mapper);
    graphics_get_api()->free(&emulator->g_ctx);
    LOG(DEBUG, "Emulator session successfully terminated");
}
