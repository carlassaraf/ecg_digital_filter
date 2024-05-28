#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "arm_math.h"

#include "app_tasks.h"

/**
 * @brief Programa principal
*/
int main(void) {
    // Array para el resultado corregidod e la RFFT
    float32_t rfft_output[FFT_LEN / 2] = {0};
    float32_t rfft_filtered[FFT_LEN / 2] = {0};
    float32_t freqs[FFT_LEN / 2] = {0};
    float32_t time[FFT_LEN] = {0};

    // Inicializacion de USB
    stdio_init_all();
    sleep_ms(2000);
    
    app_init();

    while (true) {

        // Verifico si se termino la conversion
        if(sampling_is_done()) {
            // Mando las muestras
            send_data("ifft_real", rfft_input, sizeof(rfft_input) / sizeof(float32_t));
            // Resuelvo la RFFT
            dsp_rfft(rfft_input, rfft_output, freqs, FFT_LEN, FS);
            // Aplico el filtro notch
            dsp_notch_filter(rfft_output, rfft_filtered, freqs, FFT_LEN / 2, 50.0);
            // Resuelvo la IRFFT filtrada
            dsp_irfft(rfft_filtered, rfft_input, time, FFT_LEN, FS);
            // Mando las muestras
            send_data("time", time, sizeof(time) / sizeof(float32_t));
            // Mando las muestras
            send_data("ifft_filtered", rfft_input, sizeof(rfft_input) / sizeof(float32_t));
            // Mando los resultados
            send_data("freqs", freqs, sizeof(freqs) / sizeof(float32_t));
            send_data("fft_real", rfft_output, sizeof(rfft_output) / sizeof(float32_t));
            send_data("fft_filtered", rfft_filtered, sizeof(rfft_filtered) / sizeof(float32_t));
            // Inicializo el timer otra vez
            sampling_start();
        }
    }
}
