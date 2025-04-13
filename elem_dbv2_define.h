#ifndef ELEM_DBV2_DEFINE_H
#define ELEM_DBV2_DEFINE_H

#include "elem_map_node.h"
#include <atomic>
#include <mutex>

class elem_dbv2
{
public:
    elem_dbv2(int keylen, int vallen);
    ~elem_dbv2();
    int get_frwkdb(void* requst_key, void* dst_buf);
    int instert_tonewdb(void* newkey, void* newval);
    int delete_frnewdb(void* requst_key);
    int find_frwkdb(void* requst_key);
    int reiv_towkdb(void* requst_key, void* newval);
    int db_write_check();
    void db_write_clear();
    int commit_dbrevice();


public:
    int var_mapsiz;
    int var_keylen;
    int var_vallen;
    std::mutex mtx_db_writemtx;
    std::mutex mtx_db_checkmtx;

    std::atomic<unsigned long> atm_var_requst_seccessed_times;
    std::atomic<unsigned long> atm_var_requst_failed_times;

    std::atomic<void*> atm_elem_node_working;
    std::atomic<void*> atm_elem_node_new;
    std::atomic<void*> atm_elem_node_old;
    std::atomic<bool> atm_elem_node_isold;
    elem_map_node* elem_node_work;
    elem_map_node* elem_node_old;
    elem_map_node* elem_node_new;

public:

};


#endif
