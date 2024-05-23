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

    // Inicializacion de USB
    stdio_init_all();
    sleep_ms(2000);
    
    puts("Filtro Digital para ECG!");
    app_init();

    while (true) {

        // Verifico si se termino la conversion
        if(sampling_is_done()) {
            // Resuelvo la RFFT
            solve_rfft(rfft_input, rfft_output, FFT_LEN);

            puts("\n --- Nueva muestra! --\n");
            // Recorro el array de salida
            for(uint32_t i = 0; i < FFT_LEN / 2; i++) {
                // Calculo la frecuencia de este bin
                float freq = FS * i / FFT_LEN;
                // Solo muestro las de frecuencias menores
                if(freq < 70) {
                    // Muestro el valor de amplitud y frecuencia
                    printf("%.2f @ %.2f Hz\n", rfft_output[i], freq);   
                    sleep_ms(1);
                }
                else { break; }
            }
            // Inicializo el timer otra vez
            sampling_start();
        }
    }
}
