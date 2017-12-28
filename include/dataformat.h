#ifndef _DATAFORMAT_H
#define _DATAFORMAT_H

#include <string>

namespace microdb
{

class DataFormat
{
public:
    //DataFormat() : buffer_(nullptr) { }

    bool operator==(const DataFormat& data_format);
    bool operator<(const DataFormat& data_format);
    bool operator>(const DataFormat& data_format); 

    inline void setKey(const std::string& key)
    {
        key_ = key;
    }
    
    inline void setValue(const std::string& value)
    {
        value_ = value;
    }

    inline const std::string& getKey()
    {
        return key_;
    }

    inline const std::string& getValue()
    {
        return value_;
    }

    std::string serialize();
    void deserialize(std::string buffer, int size);

    virtual ~DataFormat();
private:
    std::string int32Serialize(int32_t input);
    int32_t int32Deserialize(std::string input);

    std::string key_;
    std::string value_; 
    std::string buffer_;

    static const int32_t SIZELEN = 4;
    static const int8_t MASK = 0XFF;
    static const int8_t ZEROCHAR = 48;
};

}

#endif
