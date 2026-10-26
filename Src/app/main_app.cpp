#include <cstdio>
#include "main.h"
#include "i2c/i2c.h"
#include "lib/at24c32/at24c32.h"
#include "lib/logger/logger.h"

namespace
{
constexpr uint32_t i2c_clock_hz = 100000U;
constexpr uint32_t eeprom_timeout_ms = 100U;
constexpr uint32_t demo_log_interval_ms = 10000U;

void print_log(uint16_t number, const char *text, void *context)
{
    (void)number;
    (void)context;
    printf("%s\r\n", text);
}

void print_error(const char *operation, HAL_StatusTypeDef status)
{
    printf("[logger] %s failed (HAL status %d)\r\n",
           operation,
           static_cast<int>(status));
}
}

extern "C" void main_cpp(void)
{
    static I2C_HandleTypeDef i2c1;
    HAL_StatusTypeDef status = I2C_Init(&i2c1, I2C1, i2c_clock_hz);
    if (status != HAL_OK)
    {
        print_error("I2C1 initialization", status);
        Error_Handler();
    }

    at24c32::At24C32 eeprom(
        &i2c1,
        at24c32::default_i2c_address);
    logger::Logger log(eeprom);

    status = log.init(eeprom_timeout_ms);
    if (status != HAL_OK)
    {
        print_error("AT24C32 probe/recovery", status);
        Error_Handler();
    }

    uint16_t existing_count = 0U;
    status = log.count(&existing_count);
    if (status != HAL_OK)
    {
        print_error("log count", status);
        Error_Handler();
    }
    printf("[logger] AT24C32 ready at 0x%02X; %u log(s) recovered\r\n",
           static_cast<unsigned>(at24c32::default_i2c_address),
           static_cast<unsigned>(existing_count));

    uint16_t written_number = 0U;
    status = log.append("Info: logger started",
                        &written_number,
                        eeprom_timeout_ms);
    if (status != HAL_OK)
    {
        print_error("append startup record", status);
        Error_Handler();
    }

    status = log.append("Info: demo temperature 25C",
                        nullptr,
                        eeprom_timeout_ms);
    if (status != HAL_OK)
    {
        print_error("append sample record", status);
        Error_Handler();
    }

    status = log.append("Error: demo test event",
                        nullptr,
                        eeprom_timeout_ms);
    if (status != HAL_OK)
    {
        print_error("append test error record", status);
        Error_Handler();
    }

    uint16_t log_count = 0U;
    uint16_t first_number = 0U;
    uint16_t last_number = 0U;
    status = log.count(&log_count);
    if ((status != HAL_OK) ||
        (log.oldest_number(&first_number) != HAL_OK) ||
        (log.newest_number(&last_number) != HAL_OK))
    {
        print_error("log range query", HAL_ERROR);
        Error_Handler();
    }


    printf("[logger] count=%u, first=#%u, last=#%u\r\n",
           log_count,
           first_number,
           last_number);

    char selected_log[logger::log_text_capacity];
    status = log.read(written_number,
                      selected_log,
                      sizeof(selected_log),
                      eeprom_timeout_ms);
    if (status != HAL_OK)
    {
        print_error("read log by number", status);
        Error_Handler();
    }
    printf("[logger] read #%u: %s\r\n",
           static_cast<unsigned>(written_number),
           selected_log);

    printf("[logger] logs in recovered range:\r\n");
    status = log.read_range(first_number,
                            last_number,
                            print_log,
                            nullptr,
                            eeprom_timeout_ms);
    if (status != HAL_OK)
    {
        print_error("read log range", status);
        Error_Handler();
    }

    uint32_t last_demo_log_tick = HAL_GetTick();
    
    while (1)
    {
        if ((HAL_GetTick() - last_demo_log_tick) >= demo_log_interval_ms)
        {
            last_demo_log_tick = HAL_GetTick();
            char message[logger::log_text_capacity];
            const int length = std::snprintf(
                message,
                sizeof(message),
                "Info: uptime %lu s",
                 last_demo_log_tick / 1000U);
            if ((length < 0) ||
                (static_cast<size_t>(length) >= sizeof(message)))
            {
                print_error("format uptime record", HAL_ERROR);
                Error_Handler();
            }

            status = log.append(message, &written_number, eeprom_timeout_ms);
            if (status != HAL_OK)
            {
                print_error("append uptime record", status);
                Error_Handler();
            }

            status = log.read(written_number,
                              selected_log,
                              sizeof(selected_log),
                              eeprom_timeout_ms);
            if (status != HAL_OK)
            {
                print_error("verify uptime record", status);
                Error_Handler();
            }
            printf("[logger] %s\r\n", selected_log);
        }
        HAL_Delay(10U);
    }
    
}
