#include "dataformat.h"
#include <stdint.h>

namespace microdb
{

bool DataFormat::operator==(const DataFormat& data_format)
{
    return key_ == data_format.key_;
}

bool DataFormat::operator<(const DataFormat& data_format)
{
    return key_ < data_format.key_;
}

bool DataFormat::operator>(const DataFormat& data_format)
{
    return key_ > data_format.key_;
}

std::string DataFormat::serialize()
{
    int32_t key_size = key_.size();
    int32_t value_size = value_.size();
    buffer_.append(int32Serialize(key_size));
    buffer_.append(key_);
    buffer_.append(int32Serialize(value_size));
    buffer_.append(value_);
    return buffer_;
}

void DataFormat::deserialize(std::string buffer, int size)
{
    std::string key_size_str, value_size_str;
    key_.clear();
    value_.clear();
    int8_t i = 0;
    for (i = 0; i < SIZELEN; ++i)
    {
        key_size_str.push_back(buffer[i]);
    }
    int32_t key_size = int32Deserialize(key_size_str);
    for (i = SIZELEN; i < SIZELEN + key_size; ++i)
    {
        key_.push_back(buffer[i]);
    }
    for (i = SIZELEN + key_size; i < SIZELEN + SIZELEN + key_size; ++i)
    {
        value_size_str.push_back(buffer[i]);
    }
    int32_t value_size = int32Deserialize(value_size_str);
    for (i = SIZELEN + SIZELEN + key_size; i < SIZELEN + SIZELEN + key_size + value_size; ++i)
    {
        value_.push_back(buffer[i]);
    }
}

std::string DataFormat::int32Serialize(int32_t input)
{
    char size[SIZELEN + 1];
    int8_t i = 0;
    int8_t unit = 0;
    for (i = 0; i < SIZELEN; ++i)
    {
        unit = input & MASK;
        size[i] = (char)(unit + ZEROCHAR);
        input = input >> 8;
    }
    size[SIZELEN] = 0;
    return std::string(size);
}

int32_t DataFormat::int32Deserialize(std::string input)
{
    int32_t size = 0;
    int32_t unit = 0;
    int8_t i = 0;
    for (auto itr = input.begin(); itr != input.end(); ++itr)
    {
        unit = (int32_t)(*itr) - ZEROCHAR;
        size = size + (unit << (i * 8));
    }
    return size;
}

DataFormat::~DataFormat()
{
//    if (buffer_)
//    {
//        delete buffer_;
//        buffer_ = nullptr;
//    }
}

}
