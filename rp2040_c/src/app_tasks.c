#include <malloc.h>

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
*/
void solve_rfft(float32_t *src, float32_t *dst, uint32_t len) {
    // Reservo memoria para el resultado de la RFFT
    float32_t *raw = (float32_t*) malloc(len * sizeof(float32_t));
    
    // Calculo la RFFT
    arm_rfft_fast_f32(&rfft_instance, src, raw, 0);
    // Corrijo las magnitudes
    arm_cmplx_mag_f32(raw, dst, len / 2);
    // Escalo la salida
    for(uint32_t i = 0; i < len / 2; i++) { dst[i] /= (len / 2); }

    // Libero la memoria
    free(raw);
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