#include "logger/logger.h"

#include <cstdio>
#include <cstring>

namespace
{
constexpr uint16_t record_text_offset = 5U;
constexpr uint16_t record_text_size = 22U;
constexpr uint16_t record_crc_separator = 27U;
constexpr uint16_t record_crc_offset = 28U;
constexpr uint16_t crc16_initial = 0xFFFFU;
constexpr uint16_t crc16_polynomial = 0x1021U;
constexpr uint16_t maximum_live_number_distance = log_capacity - 1U;

uint16_t cyclicRedundancyCheck(const uint8_t *data, size_t size)
{
    uint16_t crc = crc16_initial;
    for (size_t index = 0U; index < size; ++index)
    {
        crc ^= static_cast<uint16_t>(data[index]) << 8U;
        for (uint8_t bit = 0U; bit < 8U; ++bit)
        {
            crc = (crc & 0x8000U) != 0U
                      ? static_cast<uint16_t>((crc << 1U) ^ crc16_polynomial)
                      : static_cast<uint16_t>(crc << 1U);
        }
    }
    return crc;
}

int hexDigit(uint8_t value)
{
    if ((value >= '0') && (value <= '9'))
    {
        return value - '0';
    }
    if ((value >= 'A') && (value <= 'F'))
    {
        return value - 'A' + 10;
    }
    if ((value >= 'a') && (value <= 'f'))
    {
        return value - 'a' + 10;
    }
    return -1;
}

bool decodeRecord(const uint8_t *record, uint16_t *number)
{
    if ((record[0] != '#') || (record[4] != '|') ||
        (record[27] != '|'))
    {
        return false;
    }

    uint16_t decoded_number = 0U;
    for (uint16_t index = 1U; index <= 3U; ++index)
    {
        if ((record[index] < '0') || (record[index] > '9'))
        {
            return false;
        }
        decoded_number = static_cast<uint16_t>(
            decoded_number * 10U + (record[index] - '0'));
    }

    uint16_t stored_crc = 0U;
    for (uint16_t index = 0U; index < 4U; ++index)
    {
        const int digit = hexDigit(record[record_crc_offset + index]);
        if (digit < 0)
        {
            return false;
        }
        stored_crc = static_cast<uint16_t>((stored_crc << 4U) | digit);
    }

    if ((decoded_number >= log_number_count) ||
        (stored_crc != cyclicRedundancyCheck(record, record_crc_separator + 1U)))
    {
        return false;
    }

    *number = decoded_number;
    return true;
}

void formatLogText(const uint8_t *record, uint16_t number, char *text)
{
    size_t message_length = record_text_size;
    while ((message_length > 0U) &&
           (record[record_text_offset + message_length - 1U] == ' '))
    {
        --message_length;
    }

    const int prefix_length = std::snprintf(
        text, log_text_capacity, "#%u ",
        static_cast<unsigned>(number));
    std::memcpy(text + prefix_length,
                record + record_text_offset,
                message_length);
    text[prefix_length + message_length] = '\0';
}
}

Logger::Logger(AT24C32_HandleTypeDef &eeprom)
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

bool Logger::Init()
{
    initialized_ = false;
    valid_count_ = 0U;
    next_slot_ = 0U;
    next_number_ = 0U;
    oldest_slot_ = 0U;
    newest_slot_ = 0U;
    std::memset(valid_, 0, sizeof(valid_));

    HAL_StatusTypeDef status = AT24C32_IsReady(&eeprom_);
    if (status != HAL_OK)
    {
        return false;
    }

    uint8_t record[AT24C32_PAGE_SIZE_BYTES];
    for (uint16_t slot = 0U; slot < log_capacity; ++slot)
    {
        status = AT24C32_Read(
            &eeprom_,
            static_cast<uint32_t>(slot) * AT24C32_PAGE_SIZE_BYTES,
            record,
            sizeof(record));
        if (status != HAL_OK)
        {
            return false;
        }

        uint16_t number = 0U;
        if (decodeRecord(record, &number))
        {
            valid_[slot] = true;
            numbers_[slot] = number;
            ++valid_count_;
        }
    }

    if (valid_count_ > 0U)
    {
        uint16_t best_newest_slot = 0U;
        uint16_t best_score = 0U;
        for (uint16_t candidate = 0U; candidate < log_capacity; ++candidate)
        {
            if (!valid_[candidate])
            {
                continue;
            }

            uint16_t score = 0U;
            for (uint16_t slot = 0U; slot < log_capacity; ++slot)
            {
                if (!valid_[slot])
                {
                    continue;
                }

                const uint16_t age = static_cast<uint16_t>(
                    (numbers_[candidate] + log_number_count - numbers_[slot]) %
                    log_number_count);
                if (age <= maximum_live_number_distance)
                {
                    ++score;
                }
            }

            if (score > best_score)
            {
                best_score = score;
                best_newest_slot = candidate;
            }
        }

        newest_slot_ = best_newest_slot;
        next_slot_ = static_cast<uint16_t>(
            (newest_slot_ + 1U) % log_capacity);
        next_number_ = static_cast<uint16_t>(
            (numbers_[newest_slot_] + 1U) % log_number_count);

        uint16_t oldest_age = 0U;
        oldest_slot_ = newest_slot_;
        for (uint16_t slot = 0U; slot < log_capacity; ++slot)
        {
            if (!valid_[slot])
            {
                continue;
            }

            const uint16_t age = static_cast<uint16_t>(
                (numbers_[newest_slot_] + log_number_count - numbers_[slot]) %
                log_number_count);
            if ((age <= maximum_live_number_distance) && (age > oldest_age))
            {
                oldest_age = age;
                oldest_slot_ = slot;
            }
        }
    }

    initialized_ = true;
    return true;
}

void Logger::createRecord(uint8_t *record, const char *message)
{
    std::memset(record, ' ', AT24C32_PAGE_SIZE_BYTES);
    record[0] = '#';
    record[1] = static_cast<uint8_t>('0' + next_number_ / 100U);
    record[2] = static_cast<uint8_t>('0' + (next_number_ / 10U) % 10U);
    record[3] = static_cast<uint8_t>('0' + next_number_ % 10U);
    record[4] = '|';

    for (uint16_t index = 0U;
         (index < record_text_size) && (message[index] != '\0');
         ++index)
    {
        record[record_text_offset + index] =
            static_cast<uint8_t>(message[index]);
    }
}

void Logger::addCRC(uint8_t *record)
{
    record[record_crc_separator] = '|';
    const uint16_t crc =
        cyclicRedundancyCheck(record, record_crc_separator + 1U);
    constexpr char hex[] = "0123456789ABCDEF";
    record[record_crc_offset] = hex[(crc >> 12U) & 0x0FU];
    record[record_crc_offset + 1U] = hex[(crc >> 8U) & 0x0FU];
    record[record_crc_offset + 2U] = hex[(crc >> 4U) & 0x0FU];
    record[record_crc_offset + 3U] = hex[crc & 0x0FU];
}

void Logger::updateLog()
{
    if (!valid_[next_slot_])
    {
        ++valid_count_;
    }

    valid_[next_slot_] = true;
    numbers_[next_slot_] = next_number_;
    newest_slot_ = next_slot_;

    uint16_t oldest_age = 0U;
    oldest_slot_ = newest_slot_;
    for (uint16_t slot = 0U; slot < log_capacity; ++slot)
    {
        if (!valid_[slot])
        {
            continue;
        }

        const uint16_t age = static_cast<uint16_t>(
            (numbers_[newest_slot_] + log_number_count - numbers_[slot]) %
            log_number_count);
        if ((age <= maximum_live_number_distance) && (age > oldest_age))
        {
            oldest_age = age;
            oldest_slot_ = slot;
        }
    }
}

bool Logger::Write(const char *message,
                   uint16_t *written_number)
{
    if (!initialized_ || (message == nullptr))
    {
        return false;
    }

    uint8_t record[AT24C32_PAGE_SIZE_BYTES];
    createRecord(record, message);
    addCRC(record);

    const HAL_StatusTypeDef status = AT24C32_Write(
        &eeprom_,
        static_cast<uint32_t>(next_slot_) * AT24C32_PAGE_SIZE_BYTES,
        record,
        sizeof(record));
    if (status != HAL_OK)
    {
        return false;
    }

    const uint16_t written = next_number_;
    updateLog();
    if (written_number != nullptr)
    {
        *written_number = written;
    }

    next_slot_ = static_cast<uint16_t>((next_slot_ + 1U) % log_capacity);
    next_number_ = static_cast<uint16_t>(
        (next_number_ + 1U) % log_number_count);
    return true;
}

uint16_t Logger::count() const
{
    return valid_count_;
}

bool Logger::oldest_number(uint16_t *number) const
{
    if (!initialized_ || (number == nullptr) || (valid_count_ == 0U))
    {
        return false;
    }
    *number = numbers_[oldest_slot_];
    return true;
}

bool Logger::newest_number(uint16_t *number) const
{
    if (!initialized_ || (number == nullptr) || (valid_count_ == 0U))
    {
        return false;
    }
    *number = numbers_[newest_slot_];
    return true;
}

bool Logger::findSlot(uint16_t number, uint16_t *slot) const
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

bool Logger::Read(uint16_t number, char *text, size_t text_capacity)
{
    if (!initialized_ || (text == nullptr) ||
        (text_capacity < log_text_capacity))
    {
        return false;
    }

    uint16_t slot = 0U;
    if (!findSlot(number, &slot))
    {
        return false;
    }

    uint8_t record[AT24C32_PAGE_SIZE_BYTES];
    const HAL_StatusTypeDef status = AT24C32_Read(
        &eeprom_,
        static_cast<uint32_t>(slot) * AT24C32_PAGE_SIZE_BYTES,
        record,
        sizeof(record));
    if (status != HAL_OK)
    {
        return false;
    }

    uint16_t decoded_number = 0U;
    if (!decodeRecord(record, &decoded_number) || (decoded_number != number))
    {
        return false;
    }

    formatLogText(record, number, text);
    return true;
}

bool Logger::readRange(uint16_t first_number,
                                     uint16_t last_number,
                                     LogVisitor visitor,
                                     void *context)
{
    if (!initialized_ || (visitor == nullptr) ||
        (first_number >= log_number_count) ||
        (last_number >= log_number_count))
    {
        return false;
    }

    const uint16_t range_size = static_cast<uint16_t>(
        (last_number + log_number_count - first_number) %
            log_number_count +
        1U);
    if (range_size > log_capacity)
    {
        return false;
    }

    char text[log_text_capacity];
    for (uint16_t offset = 0U; offset < range_size; ++offset)
    {
        const uint16_t number = static_cast<uint16_t>(
            (first_number + offset) % log_number_count);
        if (!Read(number, text, sizeof(text)))
        {
            return false;
        }
        visitor(number, text, context);
    }
    return true;
}

bool Logger::readAll(LogVisitor visitor, void *context)
{
    if (!initialized_ || (visitor == nullptr))
    {
        return false;
    }

    char text[log_text_capacity];
    for (uint16_t offset = 0U; offset < log_capacity; ++offset)
    {
        const uint16_t slot = static_cast<uint16_t>(
            (next_slot_ + offset) % log_capacity);
        if (!valid_[slot])
        {
            continue;
        }

        const uint16_t number = numbers_[slot];
        if (!Read(number, text, sizeof(text)))
        {
            return false;
        }
        visitor(number, text, context);
    }
    return true;
}
