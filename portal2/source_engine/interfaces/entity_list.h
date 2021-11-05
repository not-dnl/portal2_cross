#ifndef PORTAL2_CROSS_ENTITY_LIST_H
#define PORTAL2_CROSS_ENTITY_LIST_H


#include "../../../shared/memory/memory.h"

class i_entity_list
{
public:
    ADD_VFUNC(void*, this, get_client_entity(int index), 3, index);
};


#endif //PORTAL2_CROSS_ENTITY_LIST_H
