#ifndef LOGGER_H
#define LOGGER_H

#include <stddef.h>
#include <stdint.h>
#include "at24c32/at24c32.h"

constexpr uint16_t log_number_count = 1000U;
constexpr uint16_t log_capacity =
    AT24C32_SIZE_BYTES / AT24C32_PAGE_SIZE_BYTES;
const size_t log_text_capacity = 29U;

using LogVisitor = void (*)(uint16_t number, const char *text, void *context);

class Logger
{
public:
    
    Logger(AT24C32_HandleTypeDef &eeprom);

    void Init();
    void Write(const char *message,
                            uint16_t *written_number = nullptr);
    HAL_StatusTypeDef count(uint16_t *log_count) const;
    HAL_StatusTypeDef oldest_number(uint16_t *number) const;
    HAL_StatusTypeDef newest_number(uint16_t *number) const;
    HAL_StatusTypeDef Read(uint16_t number,
                           char *text,
                           size_t text_capacity) const;

    

private:
    void createRecord(uint8_t *record, const char *message);
    void addCRC(uint8_t *record);
    void updateLog();
    void nextLog();

    bool Find_Slot(uint16_t number, uint16_t *slot) const;

    AT24C32_HandleTypeDef &eeprom_;
    uint16_t numbers_[log_capacity];
    bool valid_[log_capacity];
    uint16_t valid_count_;
    uint16_t next_slot_;
    uint16_t next_number_;
    uint16_t oldest_slot_;
    uint16_t newest_slot_;
    bool initialized_;
};


#endif
