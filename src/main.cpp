
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "adc.h"

#define ADC_IN_CHANNEL_1 ADC_CHANNEL_3
#define ADC_IN_CHANNEL_2 ADC_CHANNEL_4

static const char *TAG = "MAIN";

extern "C" void app_main(void) {
    // Oneshot context for ADC_UNIT_1
    adc_drv_oneshot_ctx_t adc_ctx;
    adc_drv_oneshot_init(&adc_ctx,
                        ADC_UNIT_1,
                        ADC_RTC_CLK_SRC_DEFAULT,
                        ADC_ULP_MODE_DISABLE);

    // Channel 1 configuration (GPIO4 on ESP32-S3)
    adc_drv_oneshot_config_channel_simple(&adc_ctx,
                                            ADC_IN_CHANNEL_1,
                                            ADC_ATTEN_DB_12,
                                            ADC_BITWIDTH_DEFAULT,
                                            true);

    // Channel 2 configuration (GPIO5 on ESP32-S3)
    adc_drv_oneshot_config_channel_simple(&adc_ctx,
                                            ADC_IN_CHANNEL_2,
                                            ADC_ATTEN_DB_12,
                                            ADC_BITWIDTH_DEFAULT,
                                            true);

    while (1) {
        // Channel 1 measurements
        int raw_ch1 = 0;
        int volt_ch1 = 0;
        int volt_avg_ch1 = 0;
        adc_drv_oneshot_read_raw(&adc_ctx, ADC_IN_CHANNEL_1, &raw_ch1);
        adc_drv_oneshot_read_voltage(&adc_ctx, ADC_IN_CHANNEL_1, &volt_ch1);
        adc_drv_oneshot_read_voltage_average(&adc_ctx, ADC_IN_CHANNEL_1, 16, &volt_avg_ch1);
        float voltageCalculated = (raw_ch1 / 4095.0f) * 3.3f * 1000;
        float error =  std::abs(voltageCalculated - volt_ch1) / volt_ch1 * 100.0f;

      ESP_LOGI(TAG,
         "CH%d: Raw=%d, Volt=%d mV, VoltCalculated=%.2f mV (Error=%.2f )",
         ADC_IN_CHANNEL_1,
         raw_ch1,
         volt_ch1,
         voltageCalculated,
         error);


 

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
