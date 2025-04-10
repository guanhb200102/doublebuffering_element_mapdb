#include "elem_map_node.h"

elem_map_node::elem_map_node(int elemkey_siz, int elemvalue_siz)
{
    this->var_elemkey_siz=elemkey_siz;
    this->var_elemvalue_siz=elemvalue_siz;

    this->Node_version_No_.store(0, std::memory_order_relaxed);
    this->Node_AliveConnections.store(0, std::memory_order_relaxed);
    this->mapsiz=0;
    pthread_rwlock_init(&(this->rwlock), NULL);
}

elem_map_node::~elem_map_node()
{
    while(!this->node_map.empty())
    {
        auto it=this->node_map.begin();
        if(it->second)delete it->second;
        this->node_map.erase(it);
    }
    pthread_rwlock_destroy(&(this->rwlock));
}

int elem_map_node::node_revice(void* dstkey, void* srcvalue)
{

    elem_key d_key(dstkey, this->var_elemkey_siz);
    auto it = this->node_map.find(d_key);
    if(it!=this->node_map.end())
    {
        this->node_delete(dstkey);
    }
    this->node_insert(dstkey, srcvalue);

    return 1;
}

int elem_map_node::node_delete(void* dstkey)
{
    this->node_write_mtx.lock();
 //   pthread_rwlock_rdlock(&rwlock);
    int result;
    if(dstkey==NULL)
    {
        std::cout << "[ERROR]elem_map_node::node_delete(void* dstkey)" << std::endl;
        return 0;
    }
    elem_key d_key(dstkey, this->var_elemkey_siz);
    auto it = this->node_map.find(d_key);
    if(it!=this->node_map.end())
    {
        if(it->second)delete it->second;
        this->node_map.erase(d_key);
        result=1;
    }
    else
    {
        result=-1;
    }
 //   pthread_rwlock_unlock(&rwlock);
    this->node_write_mtx.unlock();
    return result;
}

int elem_map_node::node_insert(void* srckey, void* srcvalue)
{
    if(srckey==NULL||srcvalue==NULL)
    {
        std::cout << "[ERROR]elem_map_node::node_insert(void* srckey, void* srcvalue)" << std::endl;
        return 0;
    }
    this->node_write_mtx.lock();
  //  pthread_rwlock_rdlock(&rwlock);
    int result;
    if(this->node_isexist(srckey)==1)
    {
        result=0;
    }
    else
    {
        elem_key s_key(srckey, this->var_elemkey_siz);
        elem_value* s_value=new elem_value(srcvalue, this->var_elemvalue_siz);

        if(s_value!=NULL)
        {
            this->node_map[s_key]=s_value;
            this->mapsiz++;
            result=1;
        }
        else
        {
            std::cout << "[ERROR]memery alloc fail! new elem_value err!" << std::endl;
            result=-2;
        }
    }
 //   pthread_rwlock_wrlock(&rwlock);
    this->node_write_mtx.unlock();
    return result;
}

int elem_map_node::node_isexist(void* dstkey)
{
    if(dstkey==NULL)
    {
        std::cout << "[ERROR]elem_map_node::node_isexist(void* dstkey)" << std::endl;
        return 0;
    }
 //   pthread_rwlock_rdlock(&rwlock);
    this->Node_AliveConnections.fetch_add(1, std::memory_order_relaxed);
    int result;
    elem_key d_key(dstkey, this->var_elemkey_siz);
    auto it=this->node_map.find(d_key);
    if(it==this->node_map.end())
    {
        result=-1;
    }
    else
    {
        result=1;
    }
//   pthread_rwlock_unlock(&rwlock);
    this->Node_AliveConnections.fetch_sub(1, std::memory_order_relaxed);

    return result;
}

int elem_map_node::node_get(void* dstkey, void* dstvalue)
{
    if(dstkey==NULL||dstvalue==NULL)
    {
        std::cout << "[ERROR]elem_map_node::node_get(void* srckey, void* dstvalue)" << std::endl;
        return 0;
    }
 //   pthread_rwlock_rdlock(&rwlock);
    int result;
    this->Node_AliveConnections.fetch_add(1, std::memory_order_relaxed);
    elem_key d_key(dstkey, this->var_elemkey_siz);
    auto it=this->node_map.find(d_key);
    if(it==this->node_map.end())
    {
        result=-1;
    }
    else
    {
        if(it->second)
        {
            memcpy(dstvalue, it->second->elem_value_val, this->var_elemvalue_siz);
            result=1;
        }
        else
        {
            std::cout << "[ERROR]it->second is NULL" << std::endl;
            result=-2;
        }
    }
 //   pthread_rwlock_unlock(&rwlock);
    this->Node_AliveConnections.fetch_sub(1, std::memory_order_relaxed);

    return result;
}

unsigned short elem_map_node::elem_map_get_version()
{
    return this->Node_version_No_.load(std::memory_order_relaxed);
}

unsigned short elem_map_node::elem_map_get_conns()
{
    return this->Node_AliveConnections.load(std::memory_order_relaxed);
}

unsigned short elem_map_node::elem_map_update_version(unsigned short newno_)
{
    this->Node_version_No_.store(newno_, std::memory_order_relaxed);
    return this->Node_version_No_.load(std::memory_order_relaxed);
}
