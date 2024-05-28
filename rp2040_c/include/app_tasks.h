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
void dsp_rfft(float32_t *src, float32_t *dst, uint32_t len);
void dsp_notch_filter(float32_t *src, float32_t f0, float32_t fs, uint32_t len);
void dsp_bp_filter(float32_t *src, float32_t f1, float32_t f2, float32_t fs, uint32_t len);
void dsp_rfft_normalize(float32_t *src, float32_t *dst, uint32_t len);
void dsp_rfft_get_freq_bins(float32_t fs, uint32_t len, float32_t *dst);
void dsp_irfft(float32_t *src, float32_t *dst, uint32_t len);
void dsp_irfft_normalize(float32_t *src, float32_t *dst, uint32_t len);
void dsp_irfft_get_time_bins(float32_t fs, uint32_t len, float32_t *dst);
void send_data(char *label, float32_t *data, uint32_t len);
void sampling_start(void);
bool sampling_is_done(void);