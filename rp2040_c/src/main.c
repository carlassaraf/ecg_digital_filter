#include <stdio.h>
#include "pico/stdlib.h"
#include "arm_math.h"

#define FFT_LEN         1024UL
#define FS              500.0

#define NO_ADC

#ifdef NO_ADC
// Uso el dataset para probar
extern float32_t rfft_input[FFT_LEN];
#endif

// Array para el resultado de la RFFT
float32_t rfft_raw_output[FFT_LEN] = {0};
// Array para el resultado corregidod e la RFFT
float32_t rfft_output[FFT_LEN / 2] = {0};

/**
 * @brief Programa principal
*/
int main(void) {
    // Inicializacion de USB
    stdio_init_all();
    sleep_ms(2000);
    
    puts("Filtro Digital para ECG!");

    // Instancia para la RFFT
    arm_rfft_fast_instance_f32 rfft_instance;
    // Inicializa la RFFT
    arm_status status = arm_rfft_fast_init_f32(&rfft_instance, FFT_LEN);
    // Verifico que se haya podido inicializar
    while(status != ARM_MATH_SUCCESS);

    puts("FFT inicializada con exito!");

#ifdef NO_ADC
    // Obtengo el valor de tension para cada medicion y le saco el offset
    for(uint32_t i = 0; i < FFT_LEN; i++) { 
        rfft_input[i] = rfft_input[i] * 3.3 / 4095.0; 
    }
    puts("Valores de tension calculados!\n");
#endif

    // Calculo la RFFT
    arm_rfft_fast_f32(
        &rfft_instance,     // Instancia de RFFT
        rfft_input,         // Valores de tension del ADC
        rfft_raw_output,    // Puntero a resultado de la RFFT
        0                   // Hago RFFT
    );

    puts("FFT resuelta!\n");

    // Corrijo las magnitudes
    arm_cmplx_mag_f32(
        rfft_raw_output,
        rfft_output,
        FFT_LEN / 2
    );

    // Escalo la salida
    for(uint32_t i = 0; i < FFT_LEN / 2; i++) { 
        // Depende de la cantidad de mediciones tomadas
        rfft_output[i] /= (FFT_LEN / 2); 
    }

    puts("Magnitudes de FFT escaladas!\n");

    for(uint32_t i = 0; i < FFT_LEN / 2; i++) {

        float freq = FS * i / FFT_LEN;
        if(freq < 70) {
            printf("%.2f @ %.2f Hz\n", rfft_output[i], freq);   
            sleep_ms(1);
        }
        else { break; }
    }

    while (true);
}
