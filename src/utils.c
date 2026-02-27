#include "utils.h"
#include "platform/logging.h"
#include <stdarg.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


void LOG(LogLevel logLevel, const char* fmt, ...){
    if(TRACER)
        return;
    if(logLevel >= LOGLEVEL) {
        va_list ap;
        va_start(ap, fmt);
        logging_output(logLevel, fmt, ap);
        va_end(ap);
    }
}


void to_pixel_format(const uint32_t* restrict in, uint32_t* restrict out, size_t size, ColorFormat format){
    for(int i = 0; i < size; i++) {
        switch (format) {
            case ARGB8888:{
                out[i] = in[i];
                break;
            }
            case ABGR8888:{
                out[i] = (in[i] & 0xff000000) | ((in[i] << 16) & 0x00ff0000) | (in[i] & 0x0000ff00) | ((in[i] >> 16) & 0x000000ff);
                break;
            }
            default:
                LOG(DEBUG, "Unsupported format");
                quit(EXIT_FAILURE);
        }
    }
}

void fft(complx *v, int n, complx *tmp) {
    if(n <= 1)
        return;
    /* otherwise, do nothing and return */
    int k, m;
    complx z, w, *vo, *ve;
    ve = tmp;
    vo = tmp + n / 2;
    for (k = 0; k < n / 2; k++) {
        ve[k] = v[2 * k];
        vo[k] = v[2 * k + 1];
    }
    fft(ve, n / 2, v); /* FFT on even-indexed elements of v[] */
    fft(vo, n / 2, v); /* FFT on odd-indexed elements of v[] */
    for (m = 0; m < n / 2; m++) {
        w.Re = cos(2 * M_PI * m / (double) n);
        w.Im = -sin(2 * M_PI * m / (double) n);
        z.Re = w.Re * vo[m].Re - w.Im * vo[m].Im; /* Re(w*vo[m]) */
        z.Im = w.Re * vo[m].Im + w.Im * vo[m].Re; /* Im(w*vo[m]) */
        v[m].Re = ve[m].Re + z.Re;
        v[m].Im = ve[m].Im + z.Im;
        v[m + n / 2].Re = ve[m].Re - z.Re;
        v[m + n / 2].Im = ve[m].Im - z.Im;
    }
}


uint64_t next_power_of_2(uint64_t num) {
    int64_t power = 1;
    while(power < num)
        power*=2;
    return power;
}

void quit(int code) {
#if EXIT_PAUSE
    printf("Press Enter key to exit . . .");
    getchar();
#endif
    exit(code);
}
