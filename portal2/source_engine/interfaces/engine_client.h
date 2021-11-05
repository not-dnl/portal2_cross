#ifndef ENGINE_CLIENT_H
#define ENGINE_CLIENT_H


#include "../../../shared/memory/memory.h"

class i_engine_client
{
public:
    ADD_VFUNC(int, this, get_local_player(), 12);
};


#endif // ENGINE_CLIENT_H
