#ifndef PORTAL2_CROSS_HOOKING_MANAGER_H
#define PORTAL2_CROSS_HOOKING_MANAGER_H


#include <cstdint>
#include <vector>
#include <unordered_map>
#include "../../../shared/hook/hooking_methods.h"
#include "../../../shared/memory/memory.h"
#include "../../source_engine/other/user_cmd.h"
#include "../../../shared/hash/fnv-1a.h"
#include "../../../shared/defines/defines.h"

struct s_hook_collection;

namespace hooking_manager
{
    class c_hook_compounds;

    enum hook_type
    {
        vtable_swap
    };

    class c_hook
    {
    public:
        c_hook() = default;

        ~c_hook() = default;

        explicit c_hook(uintptr_t* instance, const hook_type& hook_type)
        {
            switch (hook_type) // if we need different hooking techniques
            {
            case vtable_swap:
                vtable_swap_hook.init(instance);
                break;
            default:
                break;
            }
        }

        hook_method::c_vtable_swap vtable_swap_hook;
        std::vector<c_hook_compounds> hook_compounds;
    };

    std::unordered_map<uintptr_t, c_hook>& get_hooks();

    s_hook_collection& get_hook_collection();

    void initialize();

    void restore();

    class c_hook_compounds
    {
    public:
        c_hook_compounds() = default;

        c_hook_compounds(const uintptr_t new_function, const int& index)
        {
            new_function_ = new_function;
            index_ = index;
        }

        ~c_hook_compounds() = default;

        template<typename Type>
        static Type create_hook(const uintptr_t& instance_hash, uintptr_t* instance, const int& index,
                const uintptr_t& new_function, const hook_type& hook_type)
        {
            auto& hooks = get_hooks();

            if (hooks.find(instance_hash) == hooks.end())
                hooks.insert(std::make_pair(instance_hash, c_hook(instance, hook_type)));

            auto& map_entry = hooks[instance_hash];
            auto& current_info = map_entry.hook_compounds.emplace_back(new_function, index);

            switch (hook_type)
            {
            case vtable_swap:
                return reinterpret_cast<Type>(map_entry.vtable_swap_hook.hook_virtual_function(
                        current_info.new_function_, current_info.index_));
            default:
                return {};
            }
        }

    private:
        uintptr_t new_function_;
        int index_;
    };
}

// huge macro that generates typedefs and the original function for us, also calls create hook that puts it in a container etc.
#define CREATE_HOOK(instance, index, new_function, hook_type) \
using type_fn_ ## new_function = decltype(&(new_function)); \
type_fn_ ## new_function o_ ## new_function = hooking_manager::c_hook_compounds::create_hook<type_fn_ ## new_function>( \
                                                            rt_hash(#instance), reinterpret_cast<uintptr_t*>(instance), \
                                                            index, reinterpret_cast<uintptr_t>(new_function), hook_type); \


inline uintptr_t* get_client_mode() // the game's client-mode interface
{
    // no need for error handling, the game will not be updated anyways
    static auto relative_call_address = memory::find_pattern(L_W("/client.so", "client.dll"),
            L_W("E8 ? ? ? ? 89 F1 8B 10", "E8 ? ? ? ? 83 3E 01"), 0,
            "client_mode");
    // we need to get the absolute address of this relative E8 call instruction
    // we add the return address to the de-referenced relative address to get it
    auto fn = reinterpret_cast<uintptr_t* (*)()>(memory::get_absolute_address(relative_call_address, 1, 5));

    return fn();
}

struct s_hook_collection
{
    static bool __fastcall create_move_hooked(HOOK_ARGS, float fl_input_sample_time, c_user_cmd* cmd);

    CREATE_HOOK(get_client_mode(), L_W(25, 24), create_move_hooked, hooking_manager::vtable_swap);
};


#endif //PORTAL2_CROSS_HOOKING_MANAGER_H
