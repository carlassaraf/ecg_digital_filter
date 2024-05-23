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
void solve_rfft(float32_t *src, float32_t *dst, uint32_t len);
void sampling_start(void);
bool sampling_is_done(void);