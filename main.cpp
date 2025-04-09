#include <iostream>
#include <cstring>
#include <cstdlib>
//#include "sqlite3_control_db_define.h"
#include <thread>
#include "elem_db_define.h"
#include <atomic>

elem_db edb(4, 4);
std::atomic<int> readnum;
void randinsert()
{
    int i=0;

    while(i<100)
    {
        edb.elem_db_insert(&i, &i);
        i++;
        sleep(1);
    }
}

void randsearch()
{
    int i=0;
    int* dstbf=new int;
    while(1)
    {
        i=0;
        while(i<50)
        {
            if(edb.elem_db_get(&i, dstbf)==1)
                readnum.fetch_add(1, std::memory_order_relaxed);

            i++;
        }
    }
}

void searall()
{

}

int main() {
    readnum.store(0, std::memory_order_relaxed);
    std::thread t(randinsert);
    std::thread t2(randsearch);
    t2.join();
}
