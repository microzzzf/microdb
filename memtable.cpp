#include "memtable.h"

namespace microdb
{

void MemTable::put(const std::string& key, const std::string& value)
{
    DataFormat data_format;
    data_format.setKey(key);
    data_format.setValue(value);
    list_.put(data_format);
}

bool MemTable::get(const std::string& key, std::string& value)
{
    DataFormat data_format;
    data_format.setKey(key);
    if (list_.get(data_format) == false)
    {
        value = "";
        return false;
    }
    else
    {
        value = data_format.getValue();
        return true;
    }
}

} //namespace microdb
