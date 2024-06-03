#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include "app_tasks.h"

// Variables publicas

// Array para las muestras
float32_t rfft_input[FFT_LEN] = {0};

// Variables privadas

// Creo un repeating timer
static repeating_timer_t timer;
// Flag para indicar que se termino el sampling
static bool sampling_done = false;

// Prototipos privados
static bool adc_start_conversion(repeating_timer_t *t);

/**
 * @brief Inicializacion de perifericos
*/
void app_init(void) {
    // Inicializacion de funciones DSP
    dsp_init();

    // Configuro el canal 0 del ADC
    adc_init();
    adc_gpio_init(ECG_ADC_GPIO);
    adc_select_input(ECG_ADC_CH);

    // Inicializo timer
    sampling_start();
}

/**
 * @brief Mando datos por USB
 * @param str cadena de texto con cadena
 * @param data puntero a datos
 * @param len cantidad de muestras
*/
void send_data(char *label, float32_t *data, uint32_t len) {
    // Reservo memoria
    char *str = (char*) malloc(12 * len + sizeof("{\"\":[]}\n") + sizeof(label));
    // Inicio de cadena
    sprintf(str, "{\"%s\":[", label);
    // Agrego cada dato
    for(uint32_t i = 0; i < len; i++) {
        char aux[10];
        // Veo si es el ultimo
        if(i < len - 1) {
            sprintf(aux, "%f,", data[i]);
        }
        else {
            sprintf(aux, "%f", data[i]);
        }
        strcat(str, aux);
    }

    sprintf(str, "%s]}\n", str);
    printf(str);
    free(str);
}

/**
 * @brief Inicializa el timer para arrancar el sampleo
*/
void sampling_start(void) {
    // Limpio el flag
    sampling_done = false;
    // Configuro un timer para asegurar la frecuencia de muestreo
    add_repeating_timer_us(
        (uint32_t)(1000000 * TS),       // Tiempo en microsegundos para el sampling rate
        adc_start_conversion,           // Callback
        NULL,                           // No hay user data
        &timer                          // Puntero a timer
    );
}

/**
 * @brief Verifica si se termino el sampling
 * @return devuelve true si se termino de muestrear
*/
bool sampling_is_done(void) {
    return sampling_done;
}

/**
 * @brief Callback del timer para hacer una conversion con el ADC
 * @param t puntero a timer usado
*/
static bool adc_start_conversion(repeating_timer_t *t) {
    // Contador de conversiones
    static uint32_t i = 0;
    // Leo el ADC, calculo la tension y saco el offset
    rfft_input[i++] = 3.3 * adc_read() / 4095;
    // Si ya se tomaron todas las muestras
    if(i == FFT_LEN) { 
        // Reinicio el contador
        i = 0; 
        // Cancelo el timer
        cancel_repeating_timer(t);
        // Seteo flag
        sampling_done = true;
    }
    return true;
}