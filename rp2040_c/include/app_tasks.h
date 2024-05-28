#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "arm_math.h"

#include "dsp.h"

#define ECG_ADC_GPIO    26
#define ECG_ADC_CH      0

// Variables para la RFFT
extern float32_t rfft_input[FFT_LEN];

// Prototipos de funciones
void app_init(void);
void send_data(char *label, float32_t *data, uint32_t len);
void sampling_start(void);
bool sampling_is_done(void);