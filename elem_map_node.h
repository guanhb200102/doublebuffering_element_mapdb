#ifndef ELEM_MAP_NODE_H
#define ELEM_MAP_NODE_H

#include <map>
#include <atomic>
#include <mutex>
#include <unistd.h>
#include <pthread.h>
#include "elem_define.h"


class elem_map_node
{
public:
    elem_map_node(int elemkey_siz, int elemvalue_siz);
    elem_map_node(elem_map_node* other);
    ~elem_map_node();
    int node_get(void* srckey, void* dstvalue);
    int node_insert(void* srckey, void* srcvalue);
    int node_delete(void* dstkey);

    int node_isexist(void* dstkey);

    unsigned short elem_map_get_conns();
    unsigned short elem_map_get_version();
    unsigned short elem_map_update_version(unsigned short newno_);
public:
    std::map<elem_key, elem_value*> node_map;
    unsigned int mapsiz;
    std::atomic<unsigned short> Node_AliveConnections;
    std::atomic<unsigned short> Node_version_No_;
    std::mutex node_write_mtx;
    elem_map_node* next;
protected:
    pthread_rwlock_t rwlock;
    int var_elemkey_siz;
    int var_elemvalue_siz;
};

#endif
