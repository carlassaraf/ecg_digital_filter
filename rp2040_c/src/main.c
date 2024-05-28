#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "arm_math.h"

#include "app_tasks.h"

/**
 * @brief Programa principal
*/
int main(void) {
    // Array para el resultado corregido de la RFFT
    float32_t rfft_output_raw[FFT_LEN] = {0};
    float32_t rfft_output_normalized[FFT_LEN / 2] = {0};
    float32_t rfft_filtered[FFT_LEN / 2] = {0};
    float32_t irfft_filtered[FFT_LEN] = {0};
    float32_t freqs[FFT_LEN / 2] = {0};
    float32_t time[FFT_LEN] = {0};

    // Inicializacion de USB
    stdio_init_all();
    sleep_ms(2000);
    
    app_init();

    while (true) {

        // Verifico si se termino la conversion
        if(sampling_is_done()) {
            // Resuelvo la RFFT
            dsp_rfft(rfft_input, rfft_output_raw, sizeof(rfft_input) / sizeof(float32_t));
            // Arreglo las magnitudes
            dsp_rfft_normalize(rfft_output_raw, rfft_output_normalized, sizeof(rfft_output_raw) / sizeof(float32_t));
            // Obtengo los bins de frecuencia
            dsp_rfft_get_freq_bins(FS, sizeof(freqs) / sizeof(float32_t), freqs);
            // Aplico el filtro notch sobre la original
            dsp_notch_filter(rfft_output_raw, 50.0, FS, sizeof(rfft_output_raw) / sizeof(float32_t));
            // Arreglo las magnitudes
            dsp_rfft_normalize(rfft_output_raw, rfft_filtered, sizeof(rfft_output_raw) / sizeof(float32_t));
            // Resuelvo la IRFFT filtrada y normalizo la salida
            dsp_irfft(rfft_filtered, irfft_filtered, sizeof(irfft_filtered) / sizeof(float32_t));
            // dsp_irfft_normalize(irfft_filtered, irfft_filtered, sizeof(irfft_filtered) / sizeof(float32_t));
            // Obtengo los bins de tiempo
            dsp_irfft_get_time_bins(FS, sizeof(time) / sizeof(float32_t), time);
            // Mando los resultados
            send_data("freqs", freqs, sizeof(freqs) / sizeof(float32_t));
            send_data("ifft_real", rfft_input, sizeof(rfft_input) / sizeof(float32_t));
            send_data("fft_real", rfft_output_normalized, sizeof(rfft_output_normalized) / sizeof(float32_t));
            send_data("time", time, sizeof(time) / sizeof(float32_t));
            send_data("ifft_filtered", irfft_filtered, sizeof(irfft_filtered) / sizeof(float32_t));
            send_data("fft_filtered", rfft_filtered, sizeof(rfft_filtered) / sizeof(float32_t));
            // Inicializo el timer otra vez
            sampling_start();
        }
    }
}
