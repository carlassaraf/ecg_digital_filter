#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include "app_tasks.h"

// Variables publicas

// Array para las muestras
float32_t rfft_input[FFT_LEN] = {0};

// Variables privadas

// Instancia para la RFFT
static arm_rfft_fast_instance_f32 rfft_instance;
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

    // Inicializa la RFFT
    arm_status status = arm_rfft_fast_init_f32(&rfft_instance, FFT_LEN);
    // Verifico que se haya podido inicializar
    while(status != ARM_MATH_SUCCESS);

    // Configuro el canal 0 del ADC
    adc_init();
    adc_gpio_init(ECG_ADC_GPIO);
    adc_select_input(ECG_ADC_CH);

    // Inicializo timer
    sampling_start();
}

/**
 * @brief Funcion que resuelve la RFFT
 * @param src puntero a muestras
 * @param dst puntero a destino de RFFT
 * @param bins puntero a frecuencias de RFFT
 * @param len cantidad de muestras
 * @param fs frecuencia de muestreo
*/
void dsp_rfft(float32_t *src, float32_t *dst, float32_t *bins, uint32_t len, float32_t fs) {
    // Reservo memoria para el resultado de la RFFT
    float32_t *raw = (float32_t*) malloc(len * sizeof(float32_t));
    
    // Calculo la RFFT
    arm_rfft_fast_f32(&rfft_instance, src, raw, 0);
    // Corrijo las magnitudes
    arm_cmplx_mag_f32(raw, dst, len / 2);
    // Escalo la salida y saco las frecuencias
    for(uint32_t i = 0; i < len / 2; i++) { 
        dst[i] /= (len / 2); 
        bins[i] = fs * i / len;
    }

    // Libero la memoria
    free(raw);
}

/**
 * @brief Funcion que aplica un filtro notch
 * @param src puntero a muestras para filtrar
 * @param dst puntero a destino del filtro
 * @param bins puntero a frecuencias
 * @param len cantidad de muestras
 * @param f0 frecuencia de resonancia del filtro
*/
void dsp_notch_filter(float32_t *src, float32_t *dst, float32_t *bins, uint32_t len, float32_t f0) {
    // Recorro todo el array de origen
    for(uint32_t i = 0; i < len; i++) {
        // Reviso si la frecuencia actual es la del filtro
        if(bins[i] > (f0 - 2.0) && bins[i] < (f0 + 2.0)) {
            // Mato armonicos en este rango
            dst[i] = 0.0;
        }
        else { dst[i] = src[i]; }
    }
}

/**
 * @brief Funcion que resuelve la RFFT
 * @param src puntero a muestras
 * @param dst puntero a destino de RFFT
 * @param bins puntero a tiempo de muestras
 * @param len cantidad de muestras
 * @param fs frecuencia de muestreo
*/
void dsp_irfft(float32_t *src, float32_t *dst, float32_t *bins, uint32_t len, float32_t fs) {
     // Calculo la IRFFT
    arm_rfft_fast_f32(&rfft_instance, src, dst, 1);
    // Calculo los bins de tiempo y ajusto la amplitud
    for(uint32_t i = 0; i < len; i++) {
        bins[i] = i / fs;
        dst[i] /= len; 
    }
}

/**
 * @brief Mando datos por USB
 * @param str cadena de texto con cadena
 * @param data puntero a datos
 * @param len cantidad de muestras
*/
void send_data(char *label, float32_t *data, uint32_t len) {
    // Reservo memoria
    char *str = (char*) malloc(8 * len + sizeof("{\"\":[]}\n") + sizeof(label));
    // Inicio de cadena
    sprintf(str, "{\"%s\":[", label);
    // Agrego cada dato
    for(uint32_t i = 0; i < len; i++) {
        char aux[10];
        // Veo si es el ultimo
        if(i < len - 1) {
            sprintf(aux, "%.2f,", data[i]);
        }
        else {
            sprintf(aux, "%.2f", data[i]);
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
    rfft_input[i++] = 3.3 * adc_read() / 4095 - 1.65;
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