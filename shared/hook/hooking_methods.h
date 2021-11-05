#ifndef PORTAL2_CROSS_HOOKING_METHODS_H
#define PORTAL2_CROSS_HOOKING_METHODS_H


#include <cstdint>
#include <cstddef>

namespace hook_method
{
    class c_vtable_swap
    {
    public:
        c_vtable_swap() = default;

        ~c_vtable_swap() = default;

        void init(uintptr_t* instance_);

        [[nodiscard]] uintptr_t hook_virtual_function(const uintptr_t& new_function, const int& index) const;

        void restore_original();

        uintptr_t* instance{};
        uintptr_t* custom_table{};
        uintptr_t* original_table_{};
    private:
        bool init_complete_{};

        [[nodiscard]] size_t count_functions() const;
    };
}


#endif //PORTAL2_CROSS_HOOKING_METHODS_H
