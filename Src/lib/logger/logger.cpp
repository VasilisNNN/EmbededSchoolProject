#include "lib/logger/logger.h"

#include <cstdio>
#include <cstring>

namespace logger
{
namespace
{
constexpr uint16_t crc16_initial = 0xFFFFU;
constexpr uint16_t crc16_polynomial = 0x1021U;
constexpr uint16_t record_prefix_size = 5U;
constexpr uint16_t record_text_offset = 5U;
constexpr uint16_t record_text_size = 22U;
constexpr uint16_t record_crc_separator = 27U;
constexpr uint16_t record_crc_offset = 28U;
constexpr uint16_t record_crc_text_size = 4U;
constexpr uint32_t records_per_number_cycle = log_number_count;
constexpr uint32_t maximum_live_number_distance = 499U;

uint16_t crc16(const uint8_t *data, size_t size)
{
    uint16_t crc = crc16_initial;
    for (size_t index = 0; index < size; ++index)
    {
        crc ^= static_cast<uint16_t>(data[index]) << 8U;
        for (uint8_t bit = 0; bit < 8U; ++bit)
        {
            crc = (crc & 0x8000U) != 0U
                      ? static_cast<uint16_t>((crc << 1U) ^ crc16_polynomial)
                      : static_cast<uint16_t>(crc << 1U);
        }
    }
    return crc;
}

uint8_t hex_value(char value)
{
    if ((value >= '0') && (value <= '9'))
    {
        return static_cast<uint8_t>(value - '0');
    }
    if ((value >= 'A') && (value <= 'F'))
    {
        return static_cast<uint8_t>(value - 'A' + 10);
    }
    if ((value >= 'a') && (value <= 'f'))
    {
        return static_cast<uint8_t>(value - 'a' + 10);
    }
    return 0xFFU;
}

bool decode_record(const uint8_t *record, uint16_t *number)
{
    if ((record == nullptr) || (number == nullptr) ||
        (record[0] != '#') || (record[4] != '|') ||
        (record[record_crc_separator] != '|'))
    {
        return false;
    }

    uint16_t decoded_number = 0U;
    for (uint16_t index = 1U; index < 4U; ++index)
    {
        if ((record[index] < '0') || (record[index] > '9'))
        {
            return false;
        }
        decoded_number = static_cast<uint16_t>(
            (decoded_number * 10U) + (record[index] - '0'));
    }

    uint16_t stored_crc = 0U;
    for (uint16_t index = 0U; index < record_crc_text_size; ++index)
    {
        const uint8_t digit = hex_value(
            static_cast<char>(record[record_crc_offset + index]));
        if (digit == 0xFFU)
        {
            return false;
        }
        stored_crc = static_cast<uint16_t>((stored_crc << 4U) | digit);
    }

    if (crc16(record, record_crc_separator + 1U) != stored_crc)
    {
        return false;
    }

    *number = decoded_number;
    return true;
}

bool is_newer(uint16_t candidate, uint16_t reference)
{
    const uint16_t distance = static_cast<uint16_t>(
        (candidate + records_per_number_cycle - reference) %
        records_per_number_cycle);
    return (distance != 0U) &&
           (distance <= maximum_live_number_distance);
}

HAL_StatusTypeDef format_log_text(const uint8_t *record,
                                  uint16_t number,
                                  char *text,
                                  size_t text_capacity)
{
    if ((text == nullptr) || (text_capacity < log_text_capacity))
    {
        return HAL_ERROR;
    }

    size_t message_length = record_text_size;
    while ((message_length > 0U) &&
           (record[record_text_offset + message_length - 1U] == ' '))
    {
        --message_length;
    }

    const int prefix_length = std::snprintf(
        text,
        text_capacity,
        "#%u ",
        static_cast<unsigned>(number));
    if ((prefix_length < 0) ||
        (static_cast<size_t>(prefix_length) + message_length + 1U >
         text_capacity))
    {
        return HAL_ERROR;
    }

    std::memcpy(text + prefix_length,
                record + record_text_offset,
                message_length);
    text[prefix_length + message_length] = '\0';
    return HAL_OK;
}
}

Logger::Logger(at24c32::At24C32 &eeprom)
    : eeprom_(eeprom),
      numbers_{},
      valid_{},
      valid_count_(0U),
      next_slot_(0U),
      next_number_(0U),
      oldest_slot_(0U),
      newest_slot_(0U),
      initialized_(false)
{
}

HAL_StatusTypeDef Logger::init(uint32_t timeout_ms)
{
    initialized_ = false;
    valid_count_ = 0U;
    next_slot_ = 0U;
    next_number_ = 0U;
    oldest_slot_ = 0U;
    newest_slot_ = 0U;
    std::memset(valid_, 0, sizeof(valid_));

    HAL_StatusTypeDef status = eeprom_.probe(timeout_ms);
    if (status != HAL_OK)
    {
        return status;
    }

    bool have_newest = false;
    for (uint16_t slot = 0U; slot < log_capacity; ++slot)
    {
        uint8_t record[at24c32::page_size];
        uint16_t number = 0U;
        status = eeprom_.read_page(
            static_cast<uint16_t>(slot * at24c32::page_size),
            record,
            timeout_ms);
        if (status != HAL_OK)
        {
            return status;
        }

        if (!decode_record(record, &number))
        {
            continue;
        }

        valid_[slot] = true;
        numbers_[slot] = number;
        ++valid_count_;

        if (!have_newest)
        {
            oldest_slot_ = slot;
            newest_slot_ = slot;
            have_newest = true;
        }
        else
        {
            if (is_newer(number, numbers_[newest_slot_]))
            {
                newest_slot_ = slot;
            }
            if (is_newer(numbers_[oldest_slot_], number))
            {
                oldest_slot_ = slot;
            }
        }
    }

    if (have_newest)
    {
        next_slot_ = static_cast<uint16_t>(
            (newest_slot_ + 1U) % log_capacity);
        next_number_ = static_cast<uint16_t>(
            (numbers_[newest_slot_] + 1U) % log_number_count);
    }

    initialized_ = true;
    return HAL_OK;
}

HAL_StatusTypeDef Logger::append(const char *message,
                                 uint16_t *written_number,
                                 uint32_t timeout_ms)
{
    if (!initialized_ || (message == nullptr))
    {
        return HAL_ERROR;
    }

    uint8_t record[at24c32::page_size];
    const int prefix_length = std::snprintf(
        reinterpret_cast<char *>(record),
        record_prefix_size + 1U,
        "#%03u|",
        static_cast<unsigned>(next_number_));
    if (prefix_length != record_prefix_size)
    {
        return HAL_ERROR;
    }

    std::memset(record + record_text_offset, ' ', record_text_size);
    const size_t message_length = std::strlen(message);
    const size_t copied_length =
        (message_length < record_text_size) ? message_length : record_text_size;
    std::memcpy(record + record_text_offset, message, copied_length);
    record[record_crc_separator] = '|';
    const uint16_t checksum = crc16(record, record_crc_separator + 1U);
    char checksum_text[record_crc_text_size + 1U];
    const int checksum_length = std::snprintf(
        checksum_text,
        sizeof(checksum_text),
        "%04X",
        static_cast<unsigned>(checksum));
    if (checksum_length != record_crc_text_size)
    {
        return HAL_ERROR;
    }
    std::memcpy(record + record_crc_offset,
                checksum_text,
                record_crc_text_size);

    const uint16_t page_address = static_cast<uint16_t>(
        next_slot_ * at24c32::page_size);
    HAL_StatusTypeDef status = eeprom_.write_page(
        page_address,
        record,
        timeout_ms);
    if (status != HAL_OK)
    {
        return status;
    }

    uint8_t verified_record[at24c32::page_size];
    status = eeprom_.read_page(page_address, verified_record, timeout_ms);
    if (status != HAL_OK)
    {
        return status;
    }

    uint16_t verified_number = 0U;
    if (!decode_record(verified_record, &verified_number) ||
        (verified_number != next_number_))
    {
        return HAL_ERROR;
    }

    const uint16_t written_slot = next_slot_;
    if (!valid_[written_slot])
    {
        ++valid_count_;
    }
    valid_[written_slot] = true;
    numbers_[written_slot] = next_number_;
    newest_slot_ = written_slot;
    if (valid_count_ == 1U)
    {
        oldest_slot_ = written_slot;
    }
    else if (valid_count_ == log_capacity)
    {
        oldest_slot_ = static_cast<uint16_t>(
            (written_slot + 1U) % log_capacity);
    }

    if (written_number != nullptr)
    {
        *written_number = next_number_;
    }
    next_slot_ = static_cast<uint16_t>(
        (next_slot_ + 1U) % log_capacity);
    next_number_ = static_cast<uint16_t>(
        (next_number_ + 1U) % log_number_count);
    return HAL_OK;
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

HAL_StatusTypeDef Logger::oldest_number(uint16_t *number) const
{
    if (!initialized_ || (number == nullptr) || (valid_count_ == 0U))
    {
        return HAL_ERROR;
    }
    *number = numbers_[oldest_slot_];
    return HAL_OK;
}

HAL_StatusTypeDef Logger::newest_number(uint16_t *number) const
{
    if (!initialized_ || (number == nullptr) || (valid_count_ == 0U))
    {
        return HAL_ERROR;
    }
    *number = numbers_[newest_slot_];
    return HAL_OK;
}

bool Logger::find_slot(uint16_t number, uint16_t *slot) const
{
    if ((number >= log_number_count) || (slot == nullptr))
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

HAL_StatusTypeDef Logger::read(uint16_t number,
                               char *text,
                               size_t text_capacity,
                               uint32_t timeout_ms) const
{
    if (!initialized_ || (text == nullptr) ||
        (text_capacity < log_text_capacity))
    {
        return HAL_ERROR;
    }

    uint16_t slot = 0U;
    if (!find_slot(number, &slot))
    {
        return HAL_ERROR;
    }

    uint8_t record[at24c32::page_size];
    HAL_StatusTypeDef status = eeprom_.read_page(
        static_cast<uint16_t>(slot * at24c32::page_size),
        record,
        timeout_ms);
    if (status != HAL_OK)
    {
        return status;
    }

    uint16_t decoded_number = 0U;
    if (!decode_record(record, &decoded_number) || (decoded_number != number))
    {
        return HAL_ERROR;
    }
    return format_log_text(record, number, text, text_capacity);
}

HAL_StatusTypeDef Logger::read_all(LogVisitor visitor,
                                   void *context,
                                   uint32_t timeout_ms) const
{
    if (!initialized_ || (visitor == nullptr))
    {
        return HAL_ERROR;
    }
    if (valid_count_ == 0U)
    {
        return HAL_OK;
    }

    for (uint16_t offset = 0U; offset < log_capacity; ++offset)
    {
        const uint16_t slot = static_cast<uint16_t>(
            (oldest_slot_ + offset) % log_capacity);
        if (!valid_[slot])
        {
            continue;
        }

        char text[log_text_capacity];
        HAL_StatusTypeDef status = read(
            numbers_[slot],
            text,
            sizeof(text),
            timeout_ms);
        if (status != HAL_OK)
        {
            return status;
        }
        visitor(numbers_[slot], text, context);
    }
    return HAL_OK;
}

HAL_StatusTypeDef Logger::read_range(uint16_t first_number,
                                     uint16_t last_number,
                                     LogVisitor visitor,
                                     void *context,
                                     uint32_t timeout_ms) const
{
    if (!initialized_ || (visitor == nullptr) ||
        (first_number >= log_number_count) ||
        (last_number >= log_number_count))
    {
        return HAL_ERROR;
    }

    const uint16_t range_size = static_cast<uint16_t>(
        ((last_number + log_number_count - first_number) %
         log_number_count) +
        1U);
    for (uint16_t offset = 0U; offset < range_size; ++offset)
    {
        const uint16_t number = static_cast<uint16_t>(
            (first_number + offset) % log_number_count);
        uint16_t slot = 0U;
        if (!find_slot(number, &slot))
        {
            continue;
        }

        char text[log_text_capacity];
        HAL_StatusTypeDef status = read(
            number,
            text,
            sizeof(text),
            timeout_ms);
        if (status != HAL_OK)
        {
            return status;
        }
        visitor(number, text, context);
    }
    return HAL_OK;
}
}
