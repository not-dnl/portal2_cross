#ifndef PORTAL2_CROSS_MEMORY_H
#define PORTAL2_CROSS_MEMORY_H


#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include "../defines/defines.h"

namespace memory
{
    bool get_module_info(const char* module_name,
            uintptr_t* address = nullptr,
            size_t* size = nullptr,
            const char** full_path = nullptr);

#ifdef _WIN32

    uintptr_t get_proc_address(const uintptr_t& module, const char* function, const bool& in_memory = false);

#endif

    std::string format_address(const uintptr_t& address);

    uintptr_t find_pattern(const char* module_name,
            const char* ida_pattern,
            const int& offset,
            const std::string& pattern_name);

    uintptr_t get_absolute_address(uintptr_t instruction, int offset, int size);

    template<typename Type, typename... Arguments>
    Type call_vfunc(void* this_ptr, int index, Arguments const& ...arguments)
    {
        return reinterpret_cast<Type(__thiscall*)(void*, Arguments...)>(
                (*reinterpret_cast<void***>(this_ptr))[index])(this_ptr,
                arguments...);
    }
}

// this macro saves us a lot of typing
#define ADD_VFUNC(type, this_ptr, function, index, ...)                  \
    type function                                                        \
    {                                                                    \
        return memory::call_vfunc<type>(this_ptr, index, ##__VA_ARGS__); \
    }


#endif //PORTAL2_CROSS_MEMORY_H