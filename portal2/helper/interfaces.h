#ifndef PORTAL2_CROSS_INTERFACES_H
#define PORTAL2_CROSS_INTERFACES_H


#include <cstdint>
#include "../source_engine/interfaces/cvar.h"
#include "../source_engine/interfaces/engine_client.h"
#include "../source_engine/interfaces/entity_list.h"
#include "../../shared/defines/defines.h"

#define ADD_INTERFACE(type, var_name, module_name, interface_name) \
    type *var_name = reinterpret_cast<type *>(                     \
        interfaces::get_interface(module_name, interface_name));

namespace interfaces
{
    uintptr_t* get_interface(const char* module_name, const char* interface_name);

    struct s_interfaces
    {
        s_interfaces() = default;

        ADD_INTERFACE(i_con_var, con_var, L_W("libvstdlib.so", "vstdlib.dll"), "VEngineCvar");
        ADD_INTERFACE(i_engine_client, engine_client, L_W("engine.so", "engine.dll"), "VEngineClient0");
        ADD_INTERFACE(i_entity_list, entity_list, L_W("/client.so", "client.dll"), "VClientEntityList0");
    };

    s_interfaces& get();
}


#endif //PORTAL2_CROSS_INTERFACES_H