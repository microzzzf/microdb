#include "skiplist.h"
#include "networker.h"
#include <sys/time.h>
#include <thread>

#define skiplist 0

int main()
{
	microdb::Networker::getInstance()->work();
	while (1)
	{	}

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
