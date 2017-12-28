#include "skiplist.h"
#include "networker.h"
#include "memtable.h"
#include <sys/time.h>
#include <thread>
#include <string>

#define skiplist 0
#define socket 0
#define memtable 0
#define dataformat 1

int main()
{
#if dataformat
    microdb::DataFormat data;
    data.setKey("zhangfan");
    data.setValue("China");
    std::cout<<data.serialize()<<std::endl;

    std::string buffer = "3000yuy4000xixi";
    data.deserialize(buffer, 0);
    std::cout<<data.getKey()<<" "<<data.getValue()<<std::endl;
#endif

#if memtable
    microdb::MemTable mem_table;
    mem_table.put("zhangfan", "China");
#endif

#if socket
    microdb::Networker::getInstance()->work();
#endif

#if skiplist
    microdb::SkipList<int> skiplist(0);
    std::thread threads[10000];
    timeval start, end;
    gettimeofday(&start, NULL); 
    for (auto i = 0; i < 10000; ++i)
        {
                threads[i] = std::thread(std::mem_fn(&microdb::SkipList<int>::put), &skiplist, i);
        }

        for (auto i = 0; i < 10000; ++i)
        {
                threads[i].join();
        }
        gettimeofday(&end, NULL);
        unsigned long time = 1000000L * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
        std::cout<<"Total insert time "<<time<<std::endl;

#if DEBUG
    skiplist.inListCount();
    std::cout<<"Inserted "<<skiplist.debug_counter.inserted_counter<<" and not inserted "<<skiplist.debug_counter.not_inserted_counter
        <<" and total "<<skiplist.debug_counter.inserted_counter + skiplist.debug_counter.not_inserted_counter
        <<" and in list "<<skiplist.debug_counter.in_list_counter<<std::endl;
#endif
#endif
    return 0;
}
