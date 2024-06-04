#include "arm_math.h"

// Definiciones

// Cantidad de muestras
#define FFT_LEN         1024UL
// Frecuencia de muestreo
#define FS              1000.0
// Tiempo de muestreo
#define TS              (1 / FS)

// Prototipos de funciones

void dsp_init(void);
void dsp_rfft(float32_t *src, float32_t *dst, uint32_t len);
void dsp_notch_filter(float32_t *src, float32_t f0, float32_t fs, uint32_t len);
void dsp_bp_filter(float32_t *src, float32_t f1, float32_t f2, float32_t fs, uint32_t len);
void dsp_rfft_normalize(float32_t *src, float32_t *dst, uint32_t len);
void dsp_irfft(float32_t *src, float32_t *dst, uint32_t len);
void dsp_irfft_normalize(float32_t *src, float32_t *dst, uint32_t len);

// Prototipos inline

/**
 * @brief Obtiene los bins de frecuencia para el espectro
 * @param fs frecuencia de muestreo
 * @param len cantidad de muestras
 * @param dst puntero a array de frecuencias
*/
static inline void dsp_rfft_get_freq_bins(float32_t fs, uint32_t len, float32_t *dst) {
    // Calculo salto de frecuencia (las muestras originales son el doble)
    const float32_t step = fs / (2 * len);
    // Calculo cada valor
    for(uint32_t i = 0; i < len; i++) { dst[i] = step * i; }
}


/**
 * @brief Obtiene los bins de tiempo para la IRFFT
 * @param fs frecuencia de muestreo
 * @param len cantidad de muestras
 * @param dst puntero a array de tiempo
*/
static inline void dsp_irfft_get_time_bins(float32_t fs, uint32_t len, float32_t *dst) {
    // Calculo cada valor
    for(uint32_t i = 0; i < len; i++) { dst[i] = i / fs; }
}