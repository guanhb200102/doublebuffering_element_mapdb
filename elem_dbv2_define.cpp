#include "elem_dbv2_define.h"

elem_dbv2::elem_dbv2(int keylen, int vallen)
{
    this->var_keylen=keylen;
    this->var_vallen=vallen;
    this->var_mapsiz=0;

    this->atm_var_requst_failed_times.store(0, std::memory_order_relaxed);
    this->atm_var_requst_seccessed_times.store(0, std::memory_order_relaxed);

    this->atm_elem_node_isold.store(false, std::memory_order_relaxed);

    this->elem_node_work=new elem_map_node(keylen, vallen);

    this->atm_elem_node_new.store(NULL, std::memory_order_relaxed);
    this->atm_elem_node_old.store(NULL, std::memory_order_relaxed);
    this->atm_elem_node_working.store(this->elem_node_work, std::memory_order_relaxed);

}

elem_dbv2::~elem_dbv2()
{
    ;
}


int elem_dbv2::db_write_check()
{
    this->mtx_db_checkmtx.lock();
    void* new_elemnode=this->atm_elem_node_new.load(std::memory_order_relaxed);
    void* work_elemnode=this->atm_elem_node_working.load(std::memory_order_relaxed);
    if(new_elemnode==NULL)
    {
        new_elemnode=new elem_map_node((elem_map_node*)work_elemnode);
        this->atm_elem_node_new.store(new_elemnode, std::memory_order_relaxed);
    }
    else {;}
    this->mtx_db_checkmtx.unlock();
    return 0;
}

int elem_dbv2::get_frwkdb(void* requst_key, void* dst_buf)
{
    if(!requst_key || !dst_buf)return -2;
    elem_map_node* wknd=(elem_map_node*)this->atm_elem_node_working.load(std::memory_order_relaxed);
    int result;
    result=wknd->node_get(requst_key, dst_buf);
    if(result==1)this->atm_var_requst_seccessed_times.fetch_add(1, std::memory_order_relaxed);
    else this->atm_var_requst_failed_times.fetch_add(1, std::memory_order_relaxed);
    return result;
}

int elem_dbv2::instert_tonewdb(void* newkey, void* newval)
{
    this->mtx_db_writemtx.lock();
    this->db_write_check();
    elem_map_node* new_elemnode=(elem_map_node*)this->atm_elem_node_new.load(std::memory_order_relaxed);
    int result = new_elemnode->node_insert(newkey, newval);
    this->var_mapsiz++;
    this->mtx_db_writemtx.unlock();
    return result;
}

int elem_dbv2::delete_frnewdb(void* requst_key)
{
    this->mtx_db_writemtx.lock();
    this->db_write_check();
    elem_map_node* new_elemnode=(elem_map_node*)this->atm_elem_node_new.load(std::memory_order_relaxed);
    int result = new_elemnode->node_delete(requst_key);
    this->var_mapsiz--;
    this->mtx_db_writemtx.unlock();
    return result;
}

int elem_dbv2::find_frwkdb(void* requst_key)
{
    char dstbuf[this->var_vallen];
    int result=this->get_frwkdb(requst_key, dstbuf);
    return result;
}

int elem_dbv2::reiv_towkdb(void* requst_key, void* newval)
{
    if(this->find_frwkdb(requst_key)==1)
    {
        this->delete_frnewdb(requst_key);
    }
    int result =
        this->instert_tonewdb(requst_key, newval);
    return result;
}

void elem_dbv2::db_write_clear()
{
    int var_chilltimes=3;
    int var_chillinterv=70000;
    int i=0;


    elem_map_node* oldnode = (elem_map_node*)this->atm_elem_node_old.load(std::memory_order_relaxed);
    unsigned short var_nodeconnections=oldnode->elem_map_get_conns();
    while(i<var_chilltimes)
    {
        while(var_nodeconnections>0)
        {
            usleep(var_chillinterv);
            var_nodeconnections=oldnode->elem_map_get_conns();
        }
        usleep(var_chillinterv);
        i++;
    }
    delete oldnode;
    this->atm_elem_node_old.store(NULL, std::memory_order_relaxed);
    return ;
}

int elem_dbv2::commit_dbrevice()
{
    this->mtx_db_checkmtx.lock();
    void* new_elemnode=this->atm_elem_node_new.load(std::memory_order_relaxed);
    void* work_elemnode=this->atm_elem_node_working.load(std::memory_order_relaxed);
    if(new_elemnode != NULL)
    {
        this->atm_elem_node_old.store(work_elemnode, std::memory_order_relaxed);
        this->atm_elem_node_working.store(new_elemnode, std::memory_order_relaxed);
        this->atm_elem_node_new.store(NULL, std::memory_order_relaxed);
        this->db_write_clear();
    }
    this->mtx_db_checkmtx.unlock();
    return 1;
}


