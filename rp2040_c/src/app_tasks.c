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
 * @param len cantidad de muestras
*/
void dsp_rfft(float32_t *src, float32_t *dst, uint32_t len) {
    // Reservo memoria para el origen de la RFFT
    float32_t *src_cpy = (float32_t*) malloc(len * sizeof(float32_t));
    // Copio los datos de src
    memcpy(src_cpy, src, len * sizeof(float32_t));
    // Calculo la RFFT
    arm_rfft_fast_f32(&rfft_instance, src_cpy, dst, 0);
    // Libero la memoria
    free(src_cpy);
}

/**
 * @brief Funcion que aplica un filtro notch (sobre la RFFT compleja)
 * @param src puntero de muestras complejas
 * @param f0 frecuencia de resonancia del filtro
 * @param fs frecuencia de muestreo
 * @param len cantidad de muestras
*/
void dsp_notch_filter(float32_t *src, float32_t f0, float32_t fs, uint32_t len) {
    // Calculo el indice de la frecuencia del filtro
    uint32_t f0_index = f0 / (fs / len);
    // Recorro todo el array de origen
    for(uint32_t i = f0_index - 5; i < f0_index + 5; i++) { 
        // Cero a la parte real y la imaginaria
        src[2 * i] = 0.0;
        src[2 * i + 1] = 0.0;
    }
}

/**
 * @brief Funcion que aplica un filtro pasabanda (sobre la RFFT compleja)
 * @param src puntero de muestras complejas
 * @param f1 frecuencia de corte inferior
 * @param f2 frecuencia de corte superior
 * @param fs frecuencia de muestreo
 * @param len cantidad de muestras
*/
void dsp_bp_filter(float32_t *src, float32_t f1, float32_t f2, float32_t fs, uint32_t len) {
    // Calculo el indice de la frecuencia inferior
    uint32_t f1_index = f1 / (fs / len);
    // Corto un poco antes el filtro
    f1_index = (f1_index < 5)? 0 : f1_index - 5;
    // Calculo el indice de la frecuencia superior y lo corto un poco despues
    uint32_t f2_index = f2 / (fs / len) + 5; 
    // Recorro todo el array de origen
    for(uint32_t i = 0; i < len; i++) { 
        // Veo si esta fuera de la banda
        if(i < f1_index || i > f2_index) {
            // Cero a la parte real y la imaginaria
            src[2 * i] = 0.0;
            src[2 * i + 1] = 0.0;
        }
    }
}

/**
 * @brief Normaliza la magnitud de la RFFT
 * @param src puntero a RFFT
 * @param dst puntero a RFFT normalizada
 * @param len cantudad de muestras
*/
void dsp_rfft_normalize(float32_t *src, float32_t *dst, uint32_t len) {
    // Copio el puntero original para no perderlo
    float32_t *src_cpy = (float32_t*) malloc(len * sizeof(float32_t));
    // Copio los datos de src
    memcpy(src_cpy, src, len * sizeof(float32_t));
    // Corrijo las magnitudes
    arm_cmplx_mag_f32(src_cpy, dst, len / 2);
    // Escalo la salida y saco las frecuencias
    for(uint32_t i = 0; i < len / 2; i++) {  dst[i] /= (len / 2);  }
    // Libero la memoria
    free(src_cpy);
}

/**
 * @brief Obtiene los bins de frecuencia para el espectro
 * @param fs frecuencia de muestreo
 * @param len cantidad de muestras
 * @param dst puntero a array de frecuencias
*/
void dsp_rfft_get_freq_bins(float32_t fs, uint32_t len, float32_t *dst) {
    // Calculo salto de frecuencia (las muestras originales son el doble)
    const float32_t step = fs / (2 * len);
    // Calculo cada valor
    for(uint32_t i = 0; i < len; i++) { dst[i] = step * i; }
}

/**
 * @brief Funcion que resuelve la RFFT
 * @param src puntero a muestras
 * @param dst puntero a destino de RFFT
 * @param len cantidad de muestras
*/
void dsp_irfft(float32_t *src, float32_t *dst, uint32_t len) {
    // Reservo memoria para el origen de la RFFT
    float32_t *src_cpy = (float32_t*) malloc(len * sizeof(float32_t));
    // Copio los datos de src
    memcpy(src_cpy, src, len * sizeof(float32_t));
    // Calculo la RFFT
    arm_rfft_fast_f32(&rfft_instance, src_cpy, dst, 1);
    // Libero la memoria
    free(src_cpy);
}

/**
 * @brief Normaliza la magnitud de la IRFFT
 * @param src puntero a IRFFT
 * @param dst puntero a IRFFT normalizada
 * @param len cantudad de muestras
*/
void dsp_irfft_normalize(float32_t *src, float32_t *dst, uint32_t len) {
    // Copio el puntero original para no perderlo
    float32_t *src_cpy = (float32_t*) malloc(len * sizeof(float32_t));
    // Copio los datos de src
    memcpy(src_cpy, src, len * sizeof(float32_t));
    // Escalo la salida y saco las frecuencias
    for(uint32_t i = 0; i < len; i++) {  dst[i] /= (len);  }
    // Libero la memoria
    free(src_cpy);
}

/**
 * @brief Obtiene los bins de tiempo para la IRFFT
 * @param fs frecuencia de muestreo
 * @param len cantidad de muestras
 * @param dst puntero a array de tiempo
*/
void dsp_irfft_get_time_bins(float32_t fs, uint32_t len, float32_t *dst) {
    // Calculo cada valor
    for(uint32_t i = 0; i < len; i++) { dst[i] = i / fs; }
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