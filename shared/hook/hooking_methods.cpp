#include <cstring>
#include "hooking_methods.h"
#include "../log/log.h"

namespace hook_method
{
    // we basically create a copy of the game's vtable
    void c_vtable_swap::init(uintptr_t* instance_)
    {
        if (!instance_)
        {
            LOG("error: failed init, due to invalid instance_ pointer");

            return;
        }

        this->instance = instance_;
        this->original_table_ = reinterpret_cast<uintptr_t*>(*instance_);

        const auto vmt_size = count_functions();
        const auto table_bytes = vmt_size * sizeof(void*);

        this->custom_table = reinterpret_cast<uintptr_t*>(malloc(table_bytes + sizeof(void*)));
        if (!this->custom_table)
        {
            LOG("error: failed init, due to invalid instance_ pointer");

            return;
        }

        // we ignore RTTI when copying the table, since it's usually not needed, if you crash and everything else is correct
        // this might be the issue ;^)
        memcpy(this->custom_table, this->original_table_, table_bytes);

        *instance_ = reinterpret_cast<uintptr_t>(this->custom_table);

        this->init_complete_ = true;
    }

    // and switch the pointers to our new function
    uintptr_t c_vtable_swap::hook_virtual_function(const uintptr_t& new_function, const int& index) const
    {
        if (!this->init_complete_)
        {
            LOG("error: failed hooking a virtual function, due to init wasn't complete");

            return 0;
        }

        this->custom_table[index] = new_function;

        return this->original_table_[index];
    }

    void c_vtable_swap::restore_original()
    {
        if (!this->original_table_)
            return;

        *this->instance = reinterpret_cast<uintptr_t>(this->original_table_);

        this->original_table_ = nullptr;
    }

    size_t c_vtable_swap::count_functions() const // warning: this is not accurate at all, but good enough.
    {
        size_t i = 0;

        while (reinterpret_cast<uintptr_t*>(*this->instance)[i])
            i++;

        return i;
    }
}