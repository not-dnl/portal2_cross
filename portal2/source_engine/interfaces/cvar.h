#ifndef PORTAL2_CROSS_CVAR_H
#define PORTAL2_CROSS_CVAR_H


#include <cstring>
#include "../../../shared/memory/memory.h"

// all those headers have rebuilt game structs or virtual functions that we can use

class c_con_var
{
public:
    void* vtable;
    c_con_var* next;
    int registered;
    char* name;
    char* help_string;
    int flags;
    void* cvar_vtable;
    c_con_var* parent;
    char* default_value;
    char* string;
    int string_length;
    float f_value;
    int i_value;
    int has_min;
    float f_min_value;
    int has_max;
    float f_max_value;
    void* change_callback;
};

class i_con_var
{
public:
    ADD_VFUNC(void, this, register_con_command(void * cvar), 12, cvar);

    ADD_VFUNC(c_con_var*, this, find_var(const char* var_name), 16, var_name);
};


#endif //PORTAL2_CROSS_CVAR_H
