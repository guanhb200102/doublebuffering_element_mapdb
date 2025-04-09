#ifndef ELEM_DB_DEFINE_H
#define ELEM_DB_DEFINE_H
#include "elem_map_node.h"

#include <atomic>
#include <mutex>
#include <unistd.h>
#include <thread>

class elem_db
{
public:
    elem_map_node* node01;
    elem_map_node* node02;
    std::atomic<unsigned short> var_db_version_index;

	std::atomic<void*> var_read_target_node;
	std::atomic<void*> var_writ_target_node;

	std::mutex db_write_mtx;

public:
    elem_db(int elemkey_siz, int elemvalue_siz);
    ~elem_db();
    int elem_db_insert(void* srckey, void* srcvalue);
    int elem_db_get(void* dstkey, void* dst_buf);
    unsigned short elem_db_versionupdate();
    unsigned short elem_db_getversions();
    int elem_db_check();

//    int elem_db_isexist(void* dstkey);
protected:
    int var_keylen;
    int var_valuelen;
};

#endif
