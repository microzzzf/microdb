#ifndef _MEMTABLE_H
#define _MEMTABLE_H

#include <string>
#include "skiplist.h"
#include "dataformat.h"

namespace microdb
{

class DataFormat;

class MemTable
{
public:
    void put(const std::string& key, const std::string& value);
    bool get(const std::string& key, std::string& value);
    std::string serialize(std::string& key);

private:
    SkipList<DataFormat> list_;
};

} //namespace microdb

#endif
