#include "esp_adc/adc_oneshot.h"
#include "ADC.h"
#include <iterator>

adc_oneshot_unit_handle_t adc_handle;
int readadcs[] = {0, 0, 0, 0, 0, 0};
int lasteadelment;
int arraylength = 6;
void ADC::Init()
{
    adc_oneshot_unit_init_cfg_t init_config = {};

    init_config.unit_id = ADC_UNIT_1;
    init_config.clk_src = ADC_RTC_CLK_SRC_DEFAULT;
    init_config.ulp_mode = ADC_ULP_MODE_DISABLE;

    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(&init_config, &adc_handle));

    adc_oneshot_chan_cfg_t config = {};

    config.bitwidth = ADC_BITWIDTH_DEFAULT;
    config.atten = ADC_ATTEN_DB_12;

    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(
            adc_handle,
            ADC_CHANNEL_3,
            &config));
}

int ADC::Read()
{
    int adc_value;
    adc_oneshot_read(adc_handle, ADC_CHANNEL_3, &adc_value);

    return adc_value;
}

int ADC::SMA()
{
    if (lasteadelment >= arraylength)
    {
        lasteadelment = 0;
    }
    readadcs[lasteadelment] = Read();

    lasteadelment++;

    int result = 0;

    for (int i = 0; i < arraylength; i++)
    {
        result += readadcs[i];
    }

    result /= arraylength;

    return result;
}
