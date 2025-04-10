#include "elem_db_define.h"

elem_db::elem_db(int elemkey_siz, int elemvalue_siz)
{
    this->var_keylen=elemkey_siz;
    this->var_valuelen=elemvalue_siz;
    this->node01 = new elem_map_node(elemkey_siz, elemvalue_siz);
    this->node02 = new elem_map_node(elemkey_siz, elemvalue_siz);
    if(this->node01==NULL || this->node02 == NULL)
    {
        std::cout << "[ERROR]elem_db::elem_db(int elemkey_siz, int elemvalue_siz)" << std::endl;
        exit(0);
    }
    this->var_db_version_index.store(0, std::memory_order_relaxed);
    this->var_read_target_node.store(this->node01);
    this->var_writ_target_node.store(this->node02);
    node01->next=node02;
    node02->next=node01;
    this->var_chilltime=100000;
}

elem_db::~elem_db()
{
    if(this->node01)
        delete this->node01;
    if(this->node02)
        delete this->node02;
}

unsigned short elem_db::elem_db_genversionno_()
{
    unsigned short cur_no_=this->elem_db_getversions();
    unsigned short new_no_;
    if(cur_no_==2000)new_no_=0;
    else new_no_=cur_no_++;
    return new_no_;
}

unsigned short elem_db::elem_db_versionupdate(unsigned short newno_)
{
    this->var_db_version_index.store(newno_, std::memory_order_relaxed);
    return this->var_db_version_index.load(std::memory_order_relaxed);
}

/*
unsigned short elem_db::elem_db_versionupdate()
{
    unsigned short cur_version=this->var_db_version_index.load(std::memory_order_relaxed);
    if(cur_version >= 2000)
        cur_version=0;
    else
        cur_version=cur_version+1;
    this->var_db_version_index.store(cur_version, std::memory_order_relaxed);
    return this->var_db_version_index.load(std::memory_order_relaxed);
}
*/

int elem_db::elem_db_revice(void* srckey, void* srcvalue)
{
    if(!srckey || !srcvalue)
        return -1;

    char dstvalue[this->var_valuelen];

    int result=this->elem_db_get(srckey, dstvalue);

    if(result==1)
        if(memcmp(srcvalue, dstvalue, this->var_valuelen)==0)
        {
            return 0;
        }
        else
        {
            this->db_write_mtx.lock();
            elem_map_node* wn=(elem_map_node*)this->var_writ_target_node.load(std::memory_order_relaxed);
            elem_map_node* rn=(elem_map_node*)this->var_read_target_node.load(std::memory_order_relaxed);

            unsigned short new_no_ = this->elem_db_genversionno_();

            while(wn->elem_map_get_conns() > 0)
                usleep(var_chilltime);
            usleep(var_chilltime);
            while(wn->elem_map_get_conns() > 0)
                usleep(var_chilltime);
            usleep(var_chilltime);


            wn->node_delete(srckey);
            wn->node_insert(srckey, srcvalue);
            wn->elem_map_update_version(new_no_);
            this->elem_db_versionupdate(new_no_);

            this->var_read_target_node.store(wn, std::memory_order_relaxed);
            this->var_writ_target_node.store(rn, std::memory_order_relaxed);

            while(rn->elem_map_get_conns()>0)
                usleep(var_chilltime);
            usleep(var_chilltime);
            while(rn->elem_map_get_conns()>0)
                usleep(var_chilltime);
            usleep(var_chilltime);


            rn->node_delete(srckey);
            rn->node_insert(srckey, srcvalue);
            rn->elem_map_update_version(new_no_);

            this->db_write_mtx.unlock();
            return 1;
        }
    else
    {
        return this->elem_db_insert(srckey, srcvalue);
    }
}

unsigned short elem_db::elem_db_getversions()
{
    return this->var_db_version_index.load(std::memory_order_relaxed);
}

int elem_db::elem_db_insert(void* srckey, void* srcvalue)
{
    if(!srckey || !srcvalue)
        return -1;
    this->db_write_mtx.lock();
    elem_map_node* wn=(elem_map_node*)this->var_writ_target_node.load(std::memory_order_relaxed);
    elem_map_node* rn=(elem_map_node*)this->var_read_target_node.load(std::memory_order_relaxed);

    while(wn->elem_map_get_conns() > 0)
        usleep(var_chilltime);
    usleep(var_chilltime);
    while(wn->elem_map_get_conns() > 0)
        usleep(var_chilltime);
    usleep(var_chilltime);


    wn->node_insert(srckey, srcvalue);

    unsigned short new_versionno_ = this->elem_db_genversionno_();

    wn->elem_map_update_version(new_versionno_);
    this->elem_db_versionupdate(new_versionno_);

    this->var_read_target_node.store(wn, std::memory_order_relaxed);

    this->var_writ_target_node.store(rn, std::memory_order_relaxed);

    while(rn->elem_map_get_conns()>0)
        usleep(var_chilltime);
    usleep(var_chilltime);
    while(rn->elem_map_get_conns()>0)
        usleep(var_chilltime);
    usleep(var_chilltime);


    rn->node_insert(srckey, srcvalue);
    rn->elem_map_update_version(new_versionno_);

    this->db_write_mtx.unlock();

    return 1;
}

int elem_db::elem_db_get(void* dstkey, void* dst_buf)
{

    if(!dstkey || !dst_buf)
        return -1;

    elem_map_node* rn = (elem_map_node*)this->var_read_target_node.load(std::memory_order_relaxed);
    while(rn->elem_map_get_version() != this->elem_db_getversions())
    {
        usleep(15);
        rn = (elem_map_node*)this->var_read_target_node.load(std::memory_order_relaxed);
    }

    return rn->node_get(dstkey, dst_buf);
}

int elem_db::elem_db_del(void* dstkey)
{
    if(dstkey==NULL)
        return -1;

    char dstbf[this->var_valuelen+1];
    if(this->elem_db_get(dstkey, dstbf)==-1)
        return -1;
    else
    {
        this->db_write_mtx.lock();
        elem_map_node* wn=(elem_map_node*)this->var_writ_target_node.load(std::memory_order_relaxed);
        elem_map_node* rn=(elem_map_node*)this->var_read_target_node.load(std::memory_order_relaxed);


        while(wn->elem_map_get_conns() > 0)
            usleep(var_chilltime);
        usleep(var_chilltime);
        while(wn->elem_map_get_conns() > 0)
            usleep(var_chilltime);
        usleep(var_chilltime);


        unsigned short new_versionno_ = this->elem_db_genversionno_();

        wn->elem_map_update_version(new_versionno_);
        this->elem_db_versionupdate(new_versionno_);

        this->var_read_target_node.store(wn, std::memory_order_relaxed);

        this->var_writ_target_node.store(rn, std::memory_order_relaxed);

        while(rn->elem_map_get_conns()>0)
            usleep(var_chilltime);
        usleep(var_chilltime);
        while(rn->elem_map_get_conns()>0)
            usleep(var_chilltime);
        usleep(var_chilltime);


        rn->node_delete(dstkey);
        rn->elem_map_update_version(new_versionno_);

        this->db_write_mtx.unlock();
        return 1;
    }
}


int elem_db::elem_db_check()
{
    this->db_write_mtx.lock();
    elem_map_node* wn=(elem_map_node*)this->var_writ_target_node.load(std::memory_order_relaxed);
    elem_map_node* rn=(elem_map_node*)this->var_read_target_node.load(std::memory_order_relaxed);
    return 1;
}
