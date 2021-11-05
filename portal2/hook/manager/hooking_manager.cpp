#include "hooking_manager.h"
#include "../../../shared/log/log.h"

namespace hooking_manager
{
    std::unordered_map<uintptr_t, c_hook>& get_hooks()
    {
        static std::unordered_map<uintptr_t, c_hook> hooks{};

        return hooks;
    }

    s_hook_collection& get_hook_collection()
    {
        static auto hook_handler = s_hook_collection();

        return hook_handler;
    }

    void initialize()
    {
        get_hook_collection();
    }

    void restore()
    {
        LOG("unhooking");

        for (auto& hook: get_hooks())
            hook.second.vtable_swap_hook.restore_original();
    }
}
