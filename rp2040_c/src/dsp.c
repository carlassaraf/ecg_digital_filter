#include <malloc.h>
#include "dsp.h"

// Instancia para la RFFT
static arm_rfft_fast_instance_f32 rfft_instance;

/**
 * @brief Inicializa lo necesario para implementar la RFFT
*/
void dsp_init(void) {
    // Inicializa la RFFT
    arm_status status = arm_rfft_fast_init_f32(&rfft_instance, FFT_LEN);
    // Verifico que se haya podido inicializar
    while(status != ARM_MATH_SUCCESS);
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