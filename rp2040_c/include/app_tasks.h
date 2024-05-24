#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "arm_math.h"

#define FFT_LEN         1024UL
// Frecuencia de muestreo
#define FS              500.0
// Tiempo de muestreo
#define TS              (1 / FS)

#define ECG_ADC_GPIO    26
#define ECG_ADC_CH      0

// Variables para la RFFT
extern float32_t rfft_input[FFT_LEN];

// Prototipos de funciones
void app_init(void);
void dsp_rfft(float32_t *src, float32_t *dst, float32_t *bins, uint32_t len, float32_t fs);
void dsp_notch_filter(float32_t *src, float32_t *dst, float32_t *bins, uint32_t len, float32_t f0);
void dsp_irfft(float32_t *src, float32_t *dst, float32_t *bins, uint32_t len, float32_t fs);
void send_data(char *label, float32_t *data, uint32_t len);
void sampling_start(void);
bool sampling_is_done(void);