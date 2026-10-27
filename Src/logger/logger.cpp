#include "logger/logger.h"

#include <cstdio>
#include <cstring>

const uint16_t crc16_initial = 0xFFFFU;
const uint16_t crc16_polynomial = 0x1021U;
const uint16_t record_text_offset = 5U;
const uint16_t record_text_size = 22U;
const uint16_t record_crc_separator = 27U;
const uint16_t record_crc_offset = 28U;
const uint16_t record_crc_text_size = 4U;
const uint32_t records_per_number_cycle = log_number_count;
const uint32_t maximum_live_number_distance = 499U;

uint16_t Cyclic_Redundancy_Check(const uint8_t *data, size_t size)
{
    uint16_t crc = crc16_initial;
    for (size_t index = 0; index < size; ++index)
    {
        crc ^= static_cast<uint16_t>(data[index]) << 8U;
        
        for (uint8_t bit = 0; bit < 8U; ++bit)
        {
            if ((crc & 0x8000U) != 0U)
            {
                crc = (crc << 1U) ^ crc16_polynomial;
            }
            else
            {
                crc = crc << 1U;
            }
        }
    }
    return crc;
}


Logger::Logger(AT24C32_HandleTypeDef &eeprom)
    : eeprom_(eeprom)
{
    
}


void Logger::Init()
{
    initialized_ = false;
    valid_count_ = 0U;
    next_slot_ = 0U;
    next_number_ = 0U;
    oldest_slot_ = 0U;
    newest_slot_ = 0U;
    std::memset(valid_, 0, sizeof(valid_));

    AT24C32_IsReady(&eeprom_);

    bool have_newest = false;

    initialized_ = true;
}

void Logger::createRecord(uint8_t *record, const char *message)
{
    for (int i = 0; i < AT24C32_PAGE_SIZE_BYTES; i++)
    {
        record[i] = ' ';
    }

    record[0] = '#';
    record[1] = '0' + next_number_ / 100;
    record[2] = '0' + (next_number_ / 10) % 10;
    record[3] = '0' + next_number_ % 10;
    record[4] = '|';

    int i = 0;

    while (message[i] != '\0' && i < record_text_size)
    {
        record[record_text_offset + i] = message[i];
        i++;
    }
}

void Logger::addCRC(uint8_t *record)
{
    record[record_crc_separator] = '|';

    uint16_t crc = Cyclic_Redundancy_Check(record, record_crc_separator + 1);

    char hex[] = "0123456789ABCDEF";

    record[record_crc_offset] = hex[(crc >> 12) & 15];
    record[record_crc_offset + 1] = hex[(crc >> 8) & 15];
    record[record_crc_offset + 2] = hex[(crc >> 4) & 15];
    record[record_crc_offset + 3] = hex[crc & 15];
}

void Logger::updateLog()
{
    if (valid_[next_slot_] == false)
    {
        valid_count_++;
    }

    valid_[next_slot_] = true;
    numbers_[next_slot_] = next_number_;

    newest_slot_ = next_slot_;

    if (valid_count_ == 1)
    {
        oldest_slot_ = next_slot_;
    }
    else if (valid_count_ == log_capacity)
    {
        oldest_slot_ = (next_slot_ + 1) % log_capacity;
    }
}

void Logger::nextLog()
{
    next_slot_++;

    if (next_slot_ >= log_capacity)
    {
        next_slot_ = 0;
    }

    next_number_++;

    if (next_number_ >= log_number_count)
    {
        next_number_ = 0;
    }
}

void Logger::Write(const char *message, uint16_t *written_number)
{
    if (initialized_ == false || message == nullptr)
    {
        return ;
    }

    uint8_t record[AT24C32_PAGE_SIZE_BYTES];

    createRecord(record, message);
    addCRC(record);

    uint16_t address = next_slot_ * AT24C32_PAGE_SIZE_BYTES;

    AT24C32_Write(
        &eeprom_,
        address,
        record,
        AT24C32_PAGE_SIZE_BYTES);

  

    updateLog();

    if (written_number != nullptr)
    {
        *written_number = next_number_;
    }

    nextLog();

}

HAL_StatusTypeDef Logger::count(uint16_t *log_count) const
{
    if (!initialized_ || (log_count == nullptr))
    {
        return HAL_ERROR;
    }
    *log_count = valid_count_;
    return HAL_OK;
}

bool Logger::Find_Slot(uint16_t number, uint16_t *slot) const
{
    if (number >= log_number_count)
    {
        return false;
    }

    for (uint16_t index = 0U; index < log_capacity; ++index)
    {
        if (valid_[index] && (numbers_[index] == number))
        {
            *slot = index;
            return true;
        }
    }
    return false;
}



void format_log_text(const uint8_t *record, uint16_t number, char *text)
{
    size_t message_length = record_text_size;
    while ((message_length > 0U) &&
           (record[record_text_offset + message_length - 1U] == ' '))
    {
        --message_length;
    }

    const int prefix_length = std::snprintf(
        text,
        log_text_capacity,
        "#%u ",
        static_cast<unsigned>(number));
    std::memcpy(text + prefix_length,
                record + record_text_offset,
                message_length);
    text[prefix_length + message_length] = '\0';
}

HAL_StatusTypeDef Logger::Read(uint16_t number,
                               char *text,
                               size_t text_capacity) const
{
    if (!initialized_ || (text == nullptr) ||
        (text_capacity < log_text_capacity))
    {
        return HAL_ERROR;
    }

    uint16_t slot = 0U;
    if (!Find_Slot(number, &slot))
    {
        return HAL_ERROR;
    }

    uint8_t record[AT24C32_PAGE_SIZE_BYTES];
    HAL_StatusTypeDef status = AT24C32_Read(
        &eeprom_,
        slot * AT24C32_PAGE_SIZE_BYTES,
        record,
        AT24C32_PAGE_SIZE_BYTES);
    if (status != HAL_OK)
    {
        return status;
    }

    uint16_t decoded_number = 0U;

    format_log_text(record, number, text);
    return HAL_OK;
}
