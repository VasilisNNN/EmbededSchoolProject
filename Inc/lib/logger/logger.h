#ifndef LOGGER_H
#define LOGGER_H

#include <stddef.h>
#include <stdint.h>
#include "lib/at24c32/at24c32.h"

namespace logger
{
constexpr uint16_t log_number_count = 1000U;
constexpr uint16_t log_capacity = at24c32::page_count;
constexpr size_t log_text_capacity = 29U;

using LogVisitor = void (*)(uint16_t number, const char *text, void *context);

class Logger
{
public:
    explicit Logger(at24c32::At24C32 &eeprom);

    HAL_StatusTypeDef init(uint32_t timeout_ms);
    HAL_StatusTypeDef append(const char *message,
                             uint16_t *written_number = nullptr,
                             uint32_t timeout_ms = 100U);
    HAL_StatusTypeDef count(uint16_t *log_count) const;
    HAL_StatusTypeDef oldest_number(uint16_t *number) const;
    HAL_StatusTypeDef newest_number(uint16_t *number) const;
    HAL_StatusTypeDef read(uint16_t number,
                           char *text,
                           size_t text_capacity,
                           uint32_t timeout_ms = 100U) const;
    HAL_StatusTypeDef read_all(LogVisitor visitor,
                               void *context,
                               uint32_t timeout_ms = 100U) const;
    HAL_StatusTypeDef read_range(uint16_t first_number,
                                 uint16_t last_number,
                                 LogVisitor visitor,
                                 void *context,
                                 uint32_t timeout_ms = 100U) const;

private:
    bool find_slot(uint16_t number, uint16_t *slot) const;

    at24c32::At24C32 &eeprom_;
    uint16_t numbers_[log_capacity];
    bool valid_[log_capacity];
    uint16_t valid_count_;
    uint16_t next_slot_;
    uint16_t next_number_;
    uint16_t oldest_slot_;
    uint16_t newest_slot_;
    bool initialized_;
};
}

#endif
