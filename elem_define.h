#ifndef ELEM_DEFINE_H
#define ELEM_DEFINE_H
#include <cstring>
#include <iostream>
#include <mutex>

class elem_key
{
public:
    int var_keysize;
    char* elem_key_val;
//    std::mutex key_write_mtx;
public:
    bool operator<(const elem_key& other) const
    {
        if(other.elem_key_val==NULL)
        {
            std::cout << "[ERROR]bool operator<(const elem_key& other) const!" <<std::endl;
            exit(0);
        }
        return std::memcmp(elem_key_val, other.elem_key_val, var_keysize) < 0;
    }

    elem_key(void* src_str, int siz)
    {
        this->var_keysize=siz;
        this->elem_key_val=new char[siz];
        if(this->elem_key_val==NULL|| src_str==NULL)
        {
            std::cout << "[ERROR]elem_key(void* src_str, int siz)!"<<std::endl;
            exit(0);
        }
        memcpy(this->elem_key_val, src_str, siz);
    }
    ~elem_key()
    {
        if(this->elem_key_val)
            delete[] this->elem_key_val;
    }

    elem_key(const elem_key& other)
    {
 //       std::cout << "elem_key(const elem_key& other)" << std::endl;
        this->var_keysize = other.var_keysize;
        this->elem_key_val = new char[this->var_keysize];
        if (this->elem_key_val == NULL)
        {
            std::cout << "[ERROR]elem_key copy constructor!" << std::endl;
            exit(0);
        }
        memcpy(this->elem_key_val, other.elem_key_val, this->var_keysize);
    }

    elem_key& operator=(const elem_key& other)
    {
        std::cout << "elem_key& operator=(const elem_key& other)" << std::endl;
        if (this != &other)
        {
            delete[] this->elem_key_val;
            this->var_keysize = other.var_keysize;
            this->elem_key_val = new char[this->var_keysize];
            if (this->elem_key_val == NULL)
            {
                std::cout << "[ERROR]elem_key assignment operator!" << std::endl;
                exit(0);
            }
            memcpy(this->elem_key_val, other.elem_key_val, this->var_keysize);
        }
        return *this;
    }
};


class elem_value
{
public:
    int var_valsiz;
    char* elem_value_val;
    std::mutex val_write_mtx;

public:
    elem_value(void* src_str, int val_siz)
    {
        this->var_valsiz=val_siz;
        this->elem_value_val=new char[val_siz];
        if( this->elem_value_val==NULL|| src_str==NULL)
        {
            std::cout << "[ERROR]elem_value(void* src_str, int val_siz)!"<<std::endl;
            exit(0);
        }
        memcpy(this->elem_value_val, src_str, val_siz);
    }

    ~elem_value()
    {
        if(this->elem_value_val)
            delete[] this->elem_value_val;
    }

};

#endif
